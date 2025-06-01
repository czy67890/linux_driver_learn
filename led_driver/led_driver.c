#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>

#define LED_GPIO1_BASE_ADDR (0xFEC200000)
#define GPIO1_MUX (0x0034 + LED_GPIO1_BASE_ADDR)
#define GPIO1_IODIR (0x000C + LED_GPIO1_BASE_ADDR)
#define GPIO1_DATA (0x0004 + LED_GPIO1_BASE_ADDR)

#define BUFFER_SIZE 1024
#define DEV_COUNT 1

static dev_t major_devno;

typedef struct __led_dev {
    struct cdev cdev;
    unsigned int __iomem *va_dr;
    unsigned int __iomem *va_ddr;
    unsigned int led_pin;
} led_dev;

struct class *led_chrdev_class;

static int led_dev_open(struct inode *inode, struct file *file)
{
    unsigned int val = 0;
    led_dev *dev = container_of(inode->i_cdev, led_dev, cdev);

    printk("led_dev_open\n");
    file->private_data = dev;

    ///set mode to output
    val = ioread32(dev->va_ddr);
    val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
    val |= ((unsigned int)(0x1) << (dev->led_pin));
    iowrite32(val, dev->va_ddr);

    val = ioread32(dev->va_dr);
    val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
    val |= ((unsigned int)(0x1) << (dev->led_pin));
    iowrite32(val, dev->va_dr);
    return 0;
}

static int led_dev_release(struct inode *inode, struct file *file)
{
    printk("chr_dev_release\n");
    return 0;
}

static ssize_t led_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long val = 0;
    char ret = 0;

    led_dev *dev = file->private_data;
    get_user(ret, buf);

    val = ioread32(dev->va_dr);
    if (ret == '0') {
        printk("off led\n");
        val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
        val &= ~((unsigned int)(0x1) << (dev->led_pin));
    } else {
        printk("on led\n");
        val |= ((unsigned int)0x01 << (dev->led_pin + 16));
        val |= ((unsigned int)0x01 << (dev->led_pin));
    }
    iowrite32(val, dev->va_dr);
    return count;
}

struct file_operations led_dev_fops = {
    .owner = THIS_MODULE,
    .open = led_dev_open,
    .release = led_dev_release,
    .write = led_dev_write,
};

static led_dev led_devs[DEV_COUNT];

static int __init led_dev_init(void)
{
    int ret = 0;
    dev_t devno = 0;
    int i = 0;

    ret = alloc_chrdev_region(&major_devno, 0, DEV_COUNT, "collin_leds");
    if (ret < 0) {
        printk("alloc dev failed \n");
        return ret;
    }

    led_devs[0].va_ddr = ioremap(GPIO1_IODIR, 4);
    led_devs[0].va_dr = ioremap(GPIO1_DATA, 4);
    led_devs[0].led_pin = 6;
    led_chrdev_class = class_create(THIS_MODULE, "collin_leds");

    for (i = 0; i < DEV_COUNT; ++i) {
        cdev_init(&led_devs[i].cdev, &led_dev_fops);

        led_devs[i].cdev.owner = THIS_MODULE;
        devno = MKDEV(MAJOR(major_devno), MINOR(major_devno) + i);
        cdev_add(&led_devs[i].cdev, devno, 1);
        device_create(led_chrdev_class, NULL, devno, NULL,
                      "collin_leds"
                      "%d",
                      i);
    }

    return ret;
}

static void __exit led_dev_exit(void)
{
    int i = 0;
    dev_t cur_devno = 0;
    printk("collin_led dev exit\n");

    for (i = 0; i < DEV_COUNT; ++i) {
        iounmap(led_devs[i].va_ddr);
        iounmap(led_devs[i].va_dr);
    }

    for (i = 0; i < DEV_COUNT; ++i) {
        cur_devno = MKDEV(MAJOR(major_devno), MINOR(major_devno) + i);
        device_destroy(led_chrdev_class, cur_devno);
        cdev_del(&led_devs[i].cdev);
    }

    unregister_chrdev_region(major_devno, DEV_COUNT);
    class_destroy(led_chrdev_class);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
