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
#include <linux/irq.h>
#include <linux/of_irq.h>

struct device_node *led_test_device_node;
struct property *led_property;

static struct class *led_class;

struct irq_test {
    dev_t control_devno;
    struct gpio_desc *control_gpio;
    struct cdev led_cdev;
    u32 interrupt_number;
};

struct irq_test control_gpio;
static irqreturn_t test_irq_hander(int irq, void *dev_id)
{
    printk("irq received\n");
    return IRQ_HANDLED;
}

static int irq_test_open(struct inode *inode, struct file *file)
{
    printk("irq_test_open\n");
    return 0;
}

static int irq_test_release(struct inode *inode, struct file *file)
{
    printk("irq_test_release\n");
    return 0;
}

static ssize_t irq_test_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char ret = '0';
    struct inode *inode = file_inode(file);
    struct irq_test *irq_test = container_of(inode->i_cdev, struct irq_test, led_cdev);

    get_user(ret, buf);
    if (ret == '0') {
        printk("irq_test_write 0\n");
        gpiod_set_value(irq_test->control_gpio, 0);
    } else {
        printk("irq_test_write 1\n");
        gpiod_set_value(irq_test->control_gpio, 1);
    }
    return count;
}

struct file_operations irq_test_fops = {
    .owner = THIS_MODULE,
    .open = irq_test_open,
    .release = irq_test_release,
    .write = irq_test_write,
};

static struct of_device_id __of_match_table[] = {{.compatible = "simulator_irq"}, {}};

static int collin_interupt_probe(struct platform_device *pdev)
{
    int error_status = -1;

    led_test_device_node = of_find_node_by_path("/simulator_irq");

    if (!led_test_device_node) {
        printk("errors");
        return -1;
    }

    printk(KERN_ALERT "name : %s", led_test_device_node->name);
    control_gpio.control_gpio = gpiod_get(&pdev->dev, "control", GPIOD_OUT_HIGH);

    if (IS_ERR(control_gpio.control_gpio)) {
        printk("gpiod_get error");
        return -1;
    }

    error_status = gpiod_direction_input(control_gpio.control_gpio);
    control_gpio.interrupt_number = irq_of_parse_and_map(led_test_device_node, 0);
    printk("\n irq_of_parse_and_map =  %d \n", control_gpio.interrupt_number);

    error_status = request_irq(control_gpio.interrupt_number, test_irq_hander, IRQF_TRIGGER_FALLING,
                               "simulator_irq", NULL);

    if (error_status != 0) {
        printk("request_irq error");
        free_irq(control_gpio.interrupt_number, NULL);
        return -1;
    }

    error_status =
        alloc_chrdev_region(&control_gpio.control_devno, 0, 1, "collin_simulator_control");

    if (error_status < 0) {
        printk(KERN_EMERG "alloc_chrdev_region failed!\n");
        return -1;
    }

    cdev_init(&(control_gpio.led_cdev), &irq_test_fops);
    control_gpio.led_cdev.owner = THIS_MODULE;

    cdev_add(&(control_gpio.led_cdev), control_gpio.control_devno, 1);

    led_class = class_create(THIS_MODULE, "collin_irq");

    device_create(led_class, NULL, control_gpio.control_devno, NULL, "collin_simulator_control");

    return 0;
}

static int collin_interupt_remove(struct platform_device *pdev)
{
    pr_info("%s\n", __func__);
    return 0;
}

static struct platform_driver irq_test_driver = {
    .probe = collin_interupt_probe,
    .remove = collin_interupt_remove,
    .driver = {.name = "collin_simulator_irq", .of_match_table = of_match_ptr(__of_match_table)}};

static int __init led_platform_driver_init(void)
{
    int DriverState;
    DriverState = platform_driver_register(&irq_test_driver);
    printk(KERN_EMERG "\tDriverState is %d\n", DriverState);
    return 0;
}

static void __exit led_platform_driver_exit(void)
{
    free_irq(control_gpio.interrupt_number, NULL);
    gpiod_put(control_gpio.control_gpio);

    device_destroy(led_class, control_gpio.control_devno);

    class_destroy(led_class);
    cdev_del(&control_gpio.led_cdev);
    unregister_chrdev_region(control_gpio.control_devno, 1);

    platform_driver_unregister(&irq_test_driver);

    printk(KERN_EMERG "dts test led exit!\n");
}

module_init(led_platform_driver_init);
module_exit(led_platform_driver_exit);

MODULE_LICENSE("GPL");
