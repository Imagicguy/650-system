#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int lkm_init(void) {
  list_del_init(&__this_module.list);
  printk(KERN_ALERT "Arciryas:module loaded\n");
  return 0;
}

static void lkm_exit(void) { printk(KERN_ALERT "Arciryas:module removed\n"); }

module_init(lkm_init);
module_exit(lkm_exit);
