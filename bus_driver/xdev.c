#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type xbus_test;

void xdev_release(struct device *dev)
{
    printk("%s-%s \n", __FILE__, __FUNCTION__);
}

static struct device xdev = {
    .init_name = "xdev",
    .bus = &xbus_test,
    .release = xdev_release,
};

unsigned long id = 0;

ssize_t xdev_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%lu\n", id);
}

ssize_t xdev_id_store(struct device *dev, struct device_attribute *attr, const char *buf,
                      size_t count)
{
    int ret = 0;
    ret = kstrtoul(buf, 1, &id);
    return count;
}

DEVICE_ATTR(xdev_id, S_IRUGO, xdev_id_show, NULL);

static __init int xdev_init(void)
{
    int ret = 0;
    printk("xdev init\n");
    ret = device_register(&xdev);
    if (ret) {
        printk("device register failed\n");
        return ret;
    }

    ret = device_create_file(&xdev, &dev_attr_xdev_id);

    if (ret) {
        device_unregister(&xdev);
        printk("create file failed\n");
    }
    return ret;
}

static __exit void xdev_exit(void)
{
    printk("xdev exit\n");
    device_remove_file(&xdev, &dev_attr_xdev_id);
    device_unregister(&xdev);
}

module_init(xdev_init);

module_exit(xdev_exit);

MODULE_LICENSE("GPL");