#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>

static struct platform_device_id led_pdev_ids[] = {{.name = "led_pdev"}, {}};

MODULE_DEVICE_TABLE(platform, led_pdev_ids);

struct led_data {
    unsigned int led_pin;
    unsigned int __iomem *mem_data;
    unsigned int __iomem *mem_dir;
    struct cdev led_dev;
};

static struct class *led_test_class;

static int led_dev_open(struct inode *inode, struct file *file)
{
    unsigned int val = 0;
    struct led_data *dev = container_of(inode->i_cdev, struct led_data, led_dev);

    printk("led_dev_open\n");
    file->private_data = dev;

    ///set mode to output
    val = ioread32(dev->mem_dir);
    val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
    val |= ((unsigned int)(0x1) << (dev->led_pin));
    iowrite32(val, dev->mem_dir);

    val = ioread32(dev->mem_data);
    val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
    val |= ((unsigned int)(0x1) << (dev->led_pin));
    iowrite32(val, dev->mem_data);
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

    struct led_data *dev = file->private_data;
    get_user(ret, buf);

    val = ioread32(dev->mem_data);
    if (ret == '0') {
        printk("off led\n");
        val |= ((unsigned int)(0x1) << (dev->led_pin + 16));
        val &= ~((unsigned int)(0x1) << (dev->led_pin));
    } else {
        printk("on led\n");
        val |= ((unsigned int)0x01 << (dev->led_pin + 16));
        val |= ((unsigned int)0x01 << (dev->led_pin));
    }
    iowrite32(val, dev->mem_data);
    return count;
}

struct file_operations led_dev_fops = {
    .owner = THIS_MODULE,
    .open = led_dev_open,
    .release = led_dev_release,
    .write = led_dev_write,
};

static int led_pdrv_probe(struct platform_device *pdev)
{
    struct led_data *cur_led;
    unsigned int *led_hwinfo;
    struct resource *mem_DR;
    struct resource *mem_DDR;

    dev_t cur_dev;
    int ret = 0;
    printk("led platform driver probe");

    cur_led = devm_kzalloc(&pdev->dev, sizeof(struct led_data), GFP_KERNEL);

    if (!cur_led) {
        return -ENOMEM;
    }

    led_hwinfo = devm_kzalloc(&pdev->dev, sizeof(unsigned int), GFP_KERNEL);

    if (!led_hwinfo) {
        return -ENOMEM;
    }

    printk("led platform inited");
    led_hwinfo = dev_get_platdata(&pdev->dev);
    cur_led->led_pin = led_hwinfo[0];

    mem_DDR = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    mem_DR = platform_get_resource(pdev, IORESOURCE_MEM, 1);

    cur_led->mem_data = devm_ioremap(&pdev->dev, mem_DR->start, resource_size(mem_DR));
    cur_led->mem_dir = devm_ioremap(&pdev->dev, mem_DDR->start, resource_size(mem_DDR));

    printk("%p-%p", cur_led->mem_data, cur_led->mem_dir);
    alloc_chrdev_region(&cur_dev, 0, 1, "led_cdev");

    cdev_init(&cur_led->led_dev, &led_dev_fops);

    ret = cdev_add(&cur_led->led_dev, cur_dev, 1);

    if (ret) {
        printk("cdev_add failed\n");
        unregister_chrdev_region(cur_dev, 1);
        return ret;
    }

    device_create(led_test_class, NULL, cur_dev, NULL, "led_cdev%d", pdev->id);

    platform_set_drvdata(pdev, cur_led);
    return 0;
}

static int led_pdrv_remove(struct platform_device *pdev)
{
    dev_t cur_dev;
    struct led_data *cur_data = platform_get_drvdata(pdev);
    printk("led platform driver remove");

    cur_dev = pdev->dev.devt;
    cdev_del(&cur_data->led_dev);
    device_destroy(led_test_class, cur_dev);
    unregister_chrdev_region(cur_dev, 1);
    return 0;
}

static struct platform_driver led_prvdrv = {.probe = led_pdrv_probe,
                                            .remove = led_pdrv_remove,
                                            .driver =
                                                {
                                                    .name = "led_pdev",
                                                    .owner = THIS_MODULE,
                                                },
                                            .id_table = led_pdev_ids};

static __init int led_pdrv_init(void)
{
    int ret = 0;
    led_test_class = class_create(THIS_MODULE, "test_leds");
    platform_driver_register(&led_prvdrv);
    return ret;
}

module_init(led_pdrv_init);

static __exit void led_pdrv_exit(void)
{
    platform_driver_unregister(&led_prvdrv);
    class_destroy(led_test_class);
}

module_exit(led_pdrv_exit);

MODULE_LICENSE("GPL");