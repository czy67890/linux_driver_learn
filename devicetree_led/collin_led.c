#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>

struct device_node *led_test_device_node;
struct property *led_property;

static struct class *led_class;

struct gpio_led {
    char *name;
    struct device_node *device_node;
    void __iomem *data_addr;
    void __iomem *dir_addr;
    unsigned pin;
    dev_t led_devno;
    struct cdev led_cdev;
};

struct gpio_led led_gpio;

static void led_gpio_on(struct gpio_led *device)
{
    unsigned int value = 0;

    printk("gpio_led_on\n");
    /* set direction */
    value = readl(device->dir_addr);
    value |= (0x1 << (device->pin + 16));
    value |= (0x1 << (device->pin));
    writel(value, device->dir_addr);

    /* open led */
    value = readl(device->data_addr);
    value |= (0x1 << (device->pin));
    value |= (0x1 << (device->pin + 16));
    writel(value, device->data_addr);
}

static void led_gpio_off(struct gpio_led *device)
{
    unsigned int value = 0;
    printk("gpio_led_off\n");
    /* off led */
    value = readl(device->data_addr);
    value &= ~(0x1 << (device->pin));
    writel(value, device->data_addr);
}

static int gpio_led_open(struct inode *inode, struct file *file)
{
    printk("gpio_led_open\n");
    return 0;
}

static int gpio_led_release(struct inode *inode, struct file *file)
{
    printk("gpio_led_release\n");
    return 0;
}

static ssize_t gpio_led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char ret = '0';
    struct inode *inode = file_inode(file);
    struct gpio_led *gpio_led = container_of(inode->i_cdev, struct gpio_led, led_cdev);

    get_user(ret, buf);
    if (ret == '0') {
        led_gpio_off(gpio_led);
    } else {
        led_gpio_on(gpio_led);
    }
    return count;
}

struct file_operations gpio_led_fops = {
    .owner = THIS_MODULE,
    .open = gpio_led_open,
    .release = gpio_led_release,
    .write = gpio_led_write,
};

static struct of_device_id __of_match_table[] = {{.compatible = "collin_led"}, {}};

static int collin_gpio_probe(struct platform_device *pdev)
{
    int error_status = -1;
    u32 reg_data[4];
    led_test_device_node = of_find_node_by_path("/collin_led");

    if (!led_test_device_node) {
        printk("errors");
        return -1;
    }

    printk(KERN_ALERT "name : %s", led_test_device_node->name);
    printk(KERN_ALERT "children name : %s", led_test_device_node->child->name);

    led_gpio.device_node = of_find_node_by_name(led_test_device_node, "led");
    if (!led_gpio.device_node) {
        printk(KERN_ALERT "errors too");
        return -1;
    }

    led_gpio.name = "collin_led0";
    printk(KERN_ALERT "name : %s", led_gpio.name);

    error_status = of_property_read_u32_array(led_gpio.device_node, "reg", reg_data, 4);

    if (error_status < 0) {
        printk("Read Error");
        return -1;
    }
    printk(KERN_ALERT "reg : %x %x %x %x", reg_data[0], reg_data[1], reg_data[2], reg_data[3]);

    led_gpio.data_addr = ioremap(reg_data[0], 4);

    if (!led_gpio.data_addr) {
        printk(KERN_ALERT "errors in ioremap");
        return -1;
    }
    led_gpio.dir_addr = ioremap(reg_data[2], 4);
    if (!led_gpio.dir_addr) {
        printk(KERN_ALERT "errors in ioremap");
        return -1;
    }

    // led_gpio.dir_addr = of_iomap(led_gpio.device_node, 1);
    // if (!led_gpio.dir_addr) {
    //     printk(KERN_ALERT "errors in io map");
    //     return -1;
    // }

    of_property_read_u32(led_gpio.device_node, "pin", &(led_gpio.pin));

    led_gpio_on(&led_gpio);

    error_status = alloc_chrdev_region(&led_gpio.led_devno, 0, 1, led_gpio.name);

    cdev_init(&(led_gpio.led_cdev), &gpio_led_fops);
    led_gpio.led_cdev.owner = THIS_MODULE;

    cdev_add(&(led_gpio.led_cdev), led_gpio.led_devno, 1);

    led_class = class_create(THIS_MODULE, "collin_led");

    device_create(led_class, NULL, led_gpio.led_devno, NULL, led_gpio.name);

    return 0;
}

static int collin_gpio_remove(struct platform_device *pdev)
{
    pr_info("%s\n", __func__);
    return 0;
}

static struct platform_driver gpio_led_driver = {
    .probe = collin_gpio_probe,
    .remove = collin_gpio_remove,
    .driver = {.name = "collin_gpio", .of_match_table = of_match_ptr(__of_match_table)}};

static int __init led_platform_driver_init(void)
{
    int DriverState;
    DriverState = platform_driver_register(&gpio_led_driver);
    printk(KERN_EMERG "\tDriverState is %d\n", DriverState);
    return 0;
}

static void __exit led_platform_driver_exit(void)
{
    iounmap(led_gpio.data_addr);
    iounmap(led_gpio.dir_addr);

    device_destroy(led_class, led_gpio.led_devno);

    class_destroy(led_class);
    cdev_del(&led_gpio.led_cdev);
    unregister_chrdev_region(led_gpio.led_devno, 1);

    platform_driver_unregister(&gpio_led_driver);

    printk(KERN_EMERG "dts test led exit!\n");
}

module_init(led_platform_driver_init);
module_exit(led_platform_driver_exit);

MODULE_LICENSE("GPL");
