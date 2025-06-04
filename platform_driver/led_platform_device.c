
#define LED_GPIO1_BASE_ADDR (0xFEC20000)
#define GPIO1_MUX (0x0034 + LED_GPIO1_BASE_ADDR)
#define GPIO1_IODIR (0x000C + LED_GPIO1_BASE_ADDR)
#define GPIO1_DATA (0x0004 + LED_GPIO1_BASE_ADDR)

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>

static struct resource led_resource[] = {
    [0] = DEFINE_RES_MEM(GPIO1_IODIR, 4), [1] = DEFINE_RES_MEM(GPIO1_DATA, 4)};

unsigned int led_hwinfo[1] = {6};

static void led_cdev_release(struct device *dev)
{
    return;
}

static struct platform_device led_pdev = {
    .name = "led_pdev",
    .id = 0,
    .resource = led_resource,
    .num_resources = ARRAY_SIZE(led_resource),
    .dev =
        {
            .release = led_cdev_release,
            .platform_data = led_hwinfo,
        },
};

static __init int led_init(void)
{
    printk("pdev inited\n");
    platform_device_register(&led_pdev);
    return 0;
}

module_init(led_init);

static __exit void led_exit(void)
{
    printk("pdev deinit\n");
    platform_device_unregister(&led_pdev);
}

module_exit(led_exit);

MODULE_LICENSE("GPL");