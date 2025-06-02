#include <linux/module.h>
#include <linux/device.h>
extern struct bus_type xbus_test;

static const char *const name = "xdrv";

int xdrv_probe(struct device *dev)
{
    printk("probing\n");
    return 0;
}

int xdrv_remove(struct device *dev)
{
    printk("removing\n");
    return 0;
}

static struct device_driver xdrv = {
    .name = "xdrv",
    .bus = &xbus_test,
    .probe = xdrv_probe,
    .remove = xdrv_remove,
};

ssize_t drvname_show(struct device_driver *drv, char *buf)
{
    return sprintf(buf, "%s\n", name);
}

DRIVER_ATTR_RO(drvname);

static __init int xdrv_init(void)
{
    int ret = 0;
    printk("xdrv inited\n");
    ret = driver_register(&xdrv);
    if (ret) {
        return ret;
    }

    ret = driver_create_file(&xdrv, &driver_attr_drvname);
    if (ret) {
        printk("create file failed\n");
        return ret;
    }
    return 0;
}

module_init(xdrv_init);

static __exit void xdrv_exit(void)
{
    driver_remove_file(&xdrv, &driver_attr_drvname);
    driver_unregister(&xdrv);
}

module_exit(xdrv_exit);

MODULE_LICENSE("GPL");