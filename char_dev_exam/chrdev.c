#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static const int dev_count = 2;

#define BUFFER_SIZE 1024

static dev_t devno;

typedef struct __char_dev_struct {
    struct cdev dev;
    char dev_buffer[BUFFER_SIZE];
} char_dev_struct;

static char_dev_struct *char_drivers = NULL;

static int chr_dev_open(struct inode *inode, struct file *file)
{
    printk("chr_dev_open\n");
    return 0;
}

static int chr_dev_release(struct inode *inode, struct file *file)
{
    printk("chr_dev_release\n");
    return 0;
}

static ssize_t chr_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret = 0;
    int tmp = count;

    struct inode *inode = file_inode(file);
    char_dev_struct *char_dev = container_of(inode->i_cdev, struct __char_dev_struct, dev);

    char *readed_buffer = char_dev->dev_buffer;
    if (p >= BUFFER_SIZE) {
        return 0;
    }

    if (tmp > BUFFER_SIZE - p) {
        tmp = BUFFER_SIZE - p;
    }

    ret = copy_to_user(buf, readed_buffer + p, tmp);
    *ppos += tmp;
    return tmp;
}

static ssize_t chr_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret = 0;
    int tmp = count;
    struct inode *inode = file_inode(file);
    char_dev_struct *char_dev = container_of(inode->i_cdev, struct __char_dev_struct, dev);

    char *write_buffer = char_dev->dev_buffer;
    if (p > BUFFER_SIZE) {
        return 0;
    }

    if (tmp > BUFFER_SIZE - p) {
        tmp = BUFFER_SIZE - p;
    }

    ret = copy_from_user(write_buffer, buf, tmp);
    *ppos += tmp;
    return tmp;
}

struct file_operations chr_dev_fops = {
    .owner = THIS_MODULE,
    .open = chr_dev_open,
    .release = chr_dev_release,
    .read = chr_dev_read,
    .write = chr_dev_write,
};

static int __init chardev_init(void)
{
    int ret = 0;
    size_t i = 0;

    printk("chardev init\n");
    char_drivers = kmalloc(sizeof(char_dev_struct) * dev_count, GFP_KERNEL);

    if (!char_drivers) {
        printk("kmalloc error\n");
        return -ENOMEM;
    }
    memset(char_drivers, 0, sizeof(char_dev_struct) * dev_count);

    ret = alloc_chrdev_region(&devno, 0, dev_count, "czy_dev");
    if (ret < 0) {
        printk("failed to alloc chrdev");
    }

    for (i = 0; i < dev_count; ++i) {
        cdev_init(&char_drivers[i].dev, &chr_dev_fops);
        ret = cdev_add(&char_drivers[i].dev, devno + i, 1);
    }

    if (ret < 0) {
        printk("cdev_add error\n");
        unregister_chrdev_region(devno, dev_count);
    }
    return ret;
}

static void __exit chardev_exit(void)
{
    int i = 0;
    printk("czy_chard dev exit\n");
    unregister_chrdev_region(devno, dev_count);
    kfree(char_drivers);
    for (i = 0; i < dev_count; ++i) {
        cdev_del(&char_drivers[i].dev);
    }
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
