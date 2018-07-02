#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/errno.h>
#include <linux/sysfs.h>
#include <linux/io.h>
#include <linux/uaccess.h>  
#include <linux/slab.h>

#define GPIO_FIRST_MINOR 0
#define GPIO_COUNT 1
#define DRIVER_NAME "ar71xx_onio"
#define CLASS_NAME "oni_gpio"
#define OAR71XX_APB_BASE 0X18000000
#define OAR71XX_GPIO_BASE OAR71XX_APB_BASE+0x00040000
#define OAR71XX_RES_SIZE 0x100
#define OAR71XX_GPIO_NUM 4
#define OAR71xx_FILE_SIZE 0x100

//RSSI1, RSSI2, RSSI3, RSSI4
int gpio_leds_offset[] = {14, 15, 22, 23};
static void __iomem *gpio_leds_addr[] = {NULL, NULL, NULL, NULL};
static char *message;
static void __iomem *gpio_base;
static struct cdev ar71xx_gpio_chardev;
dev_t device_number;
struct class *device_class;
struct device *device_file;
int gpio_open(struct inode *inode, struct file *filp);
int gpio_release(struct inode *inode, struct file *filp);
ssize_t gpio_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos);
ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *pos);

static struct file_operations ar71xx_gpio_fops = {
    //holder
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_release,
};
 
static int __init ar71xx_gpio_char_init(void) { 
    int res, i;            //save the result of each call
    pr_info("GPIO controller is going to be setup\n");
    //step 1 : register Character device driver, must include cdev
    res = alloc_chrdev_region(&device_number, GPIO_FIRST_MINOR, GPIO_COUNT, DRIVER_NAME);
    if ( res ) {
        pr_alert("Cannot allocate device number\n");
        return res;
    }

    //Step 3: Init character device
    cdev_init(&ar71xx_gpio_chardev, &ar71xx_gpio_fops);
    cdev_add(&ar71xx_gpio_chardev, device_number, GPIO_COUNT);

    //Step 2: register class
    device_class = class_create(THIS_MODULE, CLASS_NAME);
    
    if (IS_ERR(device_class))
	{
		//cdev_del(&oni_cdev);
		unregister_chrdev_region(device_number, GPIO_COUNT);
		printk(KERN_WARNING "Cannot create class");
		return PTR_ERR(device_class);
	}
	pr_info("AR71XX_GPIO: created class");

	device_file = device_create(device_class, NULL, device_number, NULL, DRIVER_NAME);
	if (IS_ERR(device_file))
	{
		class_destroy(device_class);
		//cdev_del(&oni_cdev);
		unregister_chrdev_region(device_number, GPIO_COUNT);
		printk(KERN_WARNING "Cannot create device file");
		return PTR_ERR(device_file);
	}
    pr_info("AR71XX_GPIO: created device");

    //Step 4: ioremap
    i=0;
    gpio_base = ioremap(OAR71XX_GPIO_BASE, OAR71XX_RES_SIZE);
    for (i=0; i<OAR71XX_GPIO_NUM; i++){
        gpio_leds_addr[i] = ioremap(gpio_leds_offset[i]*sizeof(u32), sizeof(u32));
        if (!gpio_leds_addr[i]){
            pr_err("OAR71XX_GPIO: Gpio number %d couldn't be allocated!!", gpio_leds_offset[i]);
            return -ENXIO;
        }
    }

    message =  kmalloc(100*sizeof(char), GFP_KERNEL);
    return 0; 
} 
 
static void __exit ar71xx_gpio_char_exit(void) { 
    pr_info("GPIO controller will be unloaded soon\n"); 
    cdev_del(&ar71xx_gpio_chardev);
    class_destroy(device_class);
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
    if (*pos >= OAR71xx_FILE_SIZE){
        pr_err("Cannot write outsite device file\n");
        return -EINVAL;
    }

    if (*pos + count >= OAR71xx_FILE_SIZE){
        count = OAR71xx_FILE_SIZE - *pos;
    }

    //Read data from user
    if (copy_from_user(message, buf, count) != 0){
        pr_err("Read data from userspace failed!");
        return -EFAULT;
    }
    pr_info("OAR71XX: read data with size %zu : %s ",count, message);
    //Compare string 

    //Final step: keep position always be 0
    *pos = 0;
    return count;
}

/*
*   Read data from device and transfer it to user space.
*/
ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *pos){
    if (*pos >= OAR71xx_FILE_SIZE){
        pr_err("Read out of range!");
        return 0;
    }

    if (*pos + count >= OAR71xx_FILE_SIZE){
        count = OAR71xx_FILE_SIZE - *pos;
    }

    pr_info("OAR71XX: Check positions: offset %zu, size %zu", *pos, count);

    if(count == 0U){
        pr_err("Nothing to read!");
        return 0;
    }

    //Read data from kernel space
    if( copy_to_user(buf, message, count) != 0){
        pr_err("Read data from kernel space failed!");
        return -EFAULT;
    }

    *pos = 0;
    return count;
}
 
module_init(ar71xx_gpio_char_init); 
module_exit(ar71xx_gpio_char_exit); 
MODULE_AUTHOR("Phi Nguyen <phind.uet@gmail.com>"); 
MODULE_LICENSE("GPL");