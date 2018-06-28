#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include < 



static int __init ar71xx_gpio_char_init(void) { 
    pr_info("GPIO controller is going to be setup\n"); 
    return 0; 
} 
 
static void __exit ar71xx_gpio_char_exit(void) { 
    pr_info("GPIO controller will be unloaded soon\n"); 
} 
 
module_init(ar71xx_gpio_char_init); 
module_exit(ar71xx_gpio_char_init); 
MODULE_AUTHOR("Phi Nguyen <phind.uet@gmail.com>"); 
MODULE_LICENSE("GPL");