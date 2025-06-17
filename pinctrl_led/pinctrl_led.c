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
    dev_t led_devno;
    struct gpio_desc *led_gpio;
    struct cdev led_cdev;
};

struct gpio_led led_gpio;

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
        gpiod_set_value(gpio_led->led_gpio, 0);
    } else {
        gpiod_set_value(gpio_led->led_gpio, 1);
    }
    return count;
}

struct file_operations gpio_led_fops = {
    .owner = THIS_MODULE,
    .open = gpio_led_open,
    .release = gpio_led_release,
    .write = gpio_led_write,
};

static struct of_device_id __of_match_table[] = {{.compatible = "collin_led_pinctrl"}, {}};

static int collin_gpio_probe(struct platform_device *pdev)
{
    int error_status = -1;
    led_test_device_node = of_find_node_by_path("/collin_led_pinctrl");

    if (!led_test_device_node) {
        printk("errors");
        return -1;
    }

    printk(KERN_ALERT "name : %s", led_test_device_node->name);

    led_gpio.name = "collin_led0";
    printk(KERN_ALERT "name : %s", led_gpio.name);

    led_gpio.led_gpio = gpiod_get(&pdev->dev, "led", GPIOD_OUT_HIGH);

    gpiod_set_value(led_gpio.led_gpio, 1);

    error_status = alloc_chrdev_region(&led_gpio.led_devno, 0, 1, led_gpio.name);
    if (error_status < 0) {
        printk(KERN_EMERG "alloc_chrdev_region failed!\n");
        return -1;
    }

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
    .driver = {.name = "collin_led_pinctrl", .of_match_table = of_match_ptr(__of_match_table)}};

static int __init led_platform_driver_init(void)
{
    int DriverState;
    DriverState = platform_driver_register(&gpio_led_driver);
    printk(KERN_EMERG "\tDriverState is %d\n", DriverState);
    return 0;
}

static void __exit led_platform_driver_exit(void)
{
    gpiod_put(led_gpio.led_gpio);

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
