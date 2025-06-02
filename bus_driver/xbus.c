#include <linux/init.h>
#include <linux/module.h>

#include <linux/device.h>

static char const *const bus_name = "xbus";
int bus_match(struct device *dev, struct device_driver *drv)
{
    printk("%s-%s\n", __FILE__, __FUNCTION__);
    if (!strncmp(dev_name(dev), drv->name, min(strlen(drv->name), strlen(dev_name(dev))))) {
        printk(" matched \n");
        return 1;
    }
    return 0;
}

struct bus_type xbus_test = {
    .name = "xbus",
    .match = bus_match,
};

ssize_t xbus_test_show(struct bus_type *bus, char *buf)
{
    return sprintf(buf, "%s\n", bus_name);
}

static struct bus_attribute xbus_test_attr = {
    .attr =
        {
            .name = "xbus_test",
            .mode = S_IRUSR,
        },
    .show = xbus_test_show,
};

static __init int xbus_init(void)
{
    int ret = 0;
    printk("xbus init\n");
    ret = bus_register(&xbus_test);

    if (ret) {
        return ret;
    }

    ret = bus_create_file(&xbus_test, &xbus_test_attr);
    if (ret) {
        bus_unregister(&xbus_test);
        return ret;
    }

    return ret;
}

module_init(xbus_init);

static __exit void xbus_exit(void)
{
    printk("xbus exit\n");
    bus_remove_file(&xbus_test, &xbus_test_attr);
    bus_unregister(&xbus_test);
}

module_exit(xbus_exit);

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(xbus_test);
