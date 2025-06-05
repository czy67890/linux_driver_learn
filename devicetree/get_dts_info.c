#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

struct device_node *led_test_device_node; //设备树节点
struct device_node *led_device_node;      //节点
struct property *led_property;            //定义属性结构体指针
static int size;

static u32 out_values[2];

static struct of_device_id __of_match_table[] = {{.compatible = "get_dts_info_test"}, {}};

static int get_dts_info_from_dts(struct platform_device *pdev)
{
    int error_status = -1;
    led_test_device_node = of_find_node_by_path("/get_dts_info_test");

    if (!led_test_device_node) {
        printk("errors");
        return -1;
    }

    printk(KERN_ALERT "name : %s", led_test_device_node->name);
    printk(KERN_ALERT "children name : %s", led_test_device_node->child->name);

    led_device_node = of_get_next_child(led_test_device_node, NULL);
    if (!led_device_node) {
        printk(KERN_ALERT "errors too");
        return -1;
    }

    led_property = of_find_property(led_device_node, "compatible", &size);
    if (!led_property) {
        printk(KERN_ALERT "error 3");
        return -1;
    }

    printk("size : %d", size);
    printk("name : %s", led_property->name);
    printk("length : %d", led_property->length);
    printk("value: %s", (char *)led_property->value);

    error_status = of_property_read_u32_array(led_device_node, "reg", out_values, 2);
    if (error_status) {
        printk("error 4");
        return -1;
    }

    printk("value1 :%d", out_values[0]);
    printk("value2 :%d", out_values[1]);
    return 0;
}

static int get_dts_info_remove(struct platform_device *pdev)
{
    pr_info("%s\n", __func__);
    return 0;
}

static struct platform_driver get_info_driver = {
    .probe = get_dts_info_from_dts,
    .remove = get_dts_info_remove,
    .driver = {.name = "get_dts_info_test", .of_match_table = of_match_ptr(__of_match_table)}};

module_platform_driver(get_info_driver);

MODULE_LICENSE("GPL");
