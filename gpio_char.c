#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/errno.h>

#define GPIO_FIRST_MINOR 0
#define GPIO_COUNT 1
#define DRIVER_NAME "ar71xx_onio"
#define CLASS_NAME "oni_gpio"
#define OAR71XX_APB_BASE 0X18000000
#define OAR71XX_GPIO_BASE OAR71XX_APB_BASE+0x00040000
#define OAR71XX_RES_SIZE 0x100

static __iomem* gpio_base;
static struct cdev ar71xx_gpio_chardev;
dev_t device_number;
class device_class;
int gpio_open(struct inode *inode, struct file *filp);
int gpio_release(struct inode *inode, struct file *filp);
ssize_t gpio_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos);
ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *pos);

static struct file_operations ar71xx_gpio_fops{
    //holder
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_release,
};
 
static int __init ar71xx_gpio_char_init(void) { 
    int res;            //save the result of each call
    pr_info("GPIO controller is going to be setup\n");
    //step 1 : register Character device driver, must include cdev
    res = alloc_chrdev_region(&device_number, GPIO_FIRST_MINOR, GPIO_COUNT, DRIVER_NAME);
    if ( res ) {
        pr_alert("Cannot allocate device number\n");
        return res;
    }

    //Step 2: register class
    device_class = class_create(THIS_MODULE, CLASS_NAME);
    if (res) {
        pr_alert("Failed to create class\n");
        class_destroy(&device_class);
        return res;
    }

    //Step 3: Init character device
    cdev_init(&ar71xx_gpio_chardev, &ar71xx_gpio_fops);
    cdev_add(&ar71xx_gpio_chardev, device_number, GPIO_COUNT);

    //Step 4: ioremap
    gpio_base = ioremap(OAR71XX_GPIO_BASE, OAR71XX_RES_SIZE);

    return 0; 
} 
 
static void __exit ar71xx_gpio_char_exit(void) { 
    pr_info("GPIO controller will be unloaded soon\n"); 
    cdev_del(&ar71xx_gpio_chardev);
    class_destroy(&device_class);
    unregister_chrdev_region(device_number, GPIO_COUNT);
} 

int gpio_open(struct inode *inode, struct file *filp){
    //Currently,there is nothing to do
    return 0;
}

int gpio_release(struct inode *inode, struct file *filp){
    //Currently,there is nothing to do
    return 0;
}

/*
*   Get data from user and write it to device.
*/
ssize_t gpio_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos){

}

/*
*   Read data from device and transfer it to user space.
*/
ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *pos){

}
 
module_init(ar71xx_gpio_char_init); 
module_exit(ar71xx_gpio_char_init); 
MODULE_AUTHOR("Phi Nguyen <phind.uet@gmail.com>"); 
MODULE_LICENSE("GPL");