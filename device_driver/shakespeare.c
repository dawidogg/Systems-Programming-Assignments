#include <linux/module.h> // well, module
#include <linux/fs.h> // registering device drivers
#include <linux/types.h> // major and minor numbers
#include <linux/cdev.h> // struct cdev and cdev_ prefixed functions
#include <linux/uaccess.h> // copy_from_user, copy_to_user
#include <linux/mutex.h> // atomic read
#include <linux/slab.h> // kmalloc kfree

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Denis Davidoglu");

#define THIS "[Shakespeare] "

static unsigned int shakespeare_major = 0;
static unsigned int shakespeare_minor = 0;
static const unsigned int shakespeare_count = 1;

struct cdev shakespeare_cdev;
struct mutex shakespeare_mutex;

static char *shakespeare_data = NULL;
static int capacity = 64;

module_param(capacity, int, S_IRUGO);

int shakespeare_open(struct inode *inode, struct file *filp) {
    printk(KERN_DEBUG THIS "Lo! Unveiling apparatus.\n");
    return 0;
}

int shakespeare_release(struct inode *inode, struct file *filp) {
    printk(KERN_DEBUG THIS "Hark! Concluding apparatus.\n");
    return 0;
}

ssize_t shakespeare_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int err, i;

    if (mutex_lock_interruptible(&shakespeare_mutex) != 0) {
        return -ERESTARTSYS;
    }

    if (*f_pos >= capacity) {
        mutex_unlock(&shakespeare_mutex);
        printk(KERN_WARNING THIS "Finale of parchment.\n");
        return 0;
    }

    if (*f_pos + count > capacity) {
        count = capacity - *f_pos;
    }

    for (i = 0; i < count; i++) {
        err = copy_to_user(buf+i, shakespeare_data + *f_pos, 1);
        if (err != 0) {
            mutex_unlock(&shakespeare_mutex);
            return -EFAULT;
        }
        *f_pos += 1;
    }
    mutex_unlock(&shakespeare_mutex);
    return count;
}

struct file_operations shakespeare_fops = {
    .owner = THIS_MODULE,
    .open = shakespeare_open,
    .release = shakespeare_release,
    .read = shakespeare_read,
    // .write = shakespeare_write,
};

static void shakespeare_fill_data(void) {
    int i = 0;
    const char poem[] = 
        "Shall I compare thee to a summer's day?\n"
        "Thou art more lovely and more temperate:\n"
        "Rough winds do shake the darling buds of May,\n"
        "And summer's lease hath all too short a date\n";
    const int poem_size = sizeof(poem)/sizeof(char)-1;
    for (i = 0; i < capacity; i++) {
        shakespeare_data[i] = poem[i%poem_size];
    }
}

static int shakespeare_init(void) {
    dev_t devno = 0;
    int result = -1;
    int err = 0;

    result = alloc_chrdev_region(&devno, shakespeare_minor, shakespeare_count, "shakespeare");
    shakespeare_major = MAJOR(devno);
    if (result < 0) {
        printk(KERN_WARNING THIS "Perplexed by num'rous mark.\n");
        return result;
    }
    
    mutex_init(&shakespeare_mutex);
    cdev_init(&shakespeare_cdev, &shakespeare_fops);
    err = cdev_add(&shakespeare_cdev, devno, shakespeare_count);
    if (err != 0) {
        printk(KERN_NOTICE THIS "Mishap %d encountered in apparatus addition.\n", err);
    }

    shakespeare_data = kmalloc(sizeof(char)*capacity, GFP_KERNEL);
    if (shakespeare_data == NULL)
        return -ENOMEM;
    shakespeare_fill_data();

    printk(KERN_INFO THIS "Greetings, thou wide world.\n");
    printk(KERN_INFO THIS "Major: %d, Minor: %d\n", shakespeare_major, shakespeare_minor);
    printk(KERN_INFO THIS "Capacity: %d\n", capacity);
    return 0;
}

static void shakespeare_exit(void) {
    dev_t devno = 0;
    cdev_del(&shakespeare_cdev);
    devno = MKDEV(shakespeare_major, shakespeare_minor);
    unregister_chrdev_region(devno, shakespeare_count);
    kfree(shakespeare_data);
    printk(KERN_INFO THIS "God save your majesty\n");
}

module_init(shakespeare_init);
module_exit(shakespeare_exit);
