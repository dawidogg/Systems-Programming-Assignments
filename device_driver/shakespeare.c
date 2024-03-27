#include <linux/module.h> // well, module
#include <linux/fs.h> // registering device drivers
#include <linux/types.h> // major and minor numbers
#include <linux/cdev.h> // struct cdev and cdev_ prefixed functions
#include <linux/uaccess.h> // copy_from_user, copy_to_user
#include <linux/mutex.h> // atomic read
#include <linux/slab.h> // kmalloc kfree
#include <linux/errno.h> // error codes like -EFAULT 
#include <linux/ioctl.h> // I/O control
#include <linux/device.h> // Automatic device node creation
#include <linux/kdev_t.h>
#include <linux/proc_fs.h> // for adding device entry under proc
#include <linux/kernel.h>

#define THIS "[Shakespeare] "

#define SHAKESPEARE_IOC_MAGIC 'S'
#define SHAKESPEARE_INC _IOW(SHAKESPEARE_IOC_MAGIC, 1, signed char)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Denis Davidoglu");

static unsigned int shakespeare_major = 0;
static unsigned int shakespeare_minor = 0;
static const unsigned int shakespeare_count = 1;
struct cdev shakespeare_cdev;
struct mutex shakespeare_mutex;
static char *shakespeare_data = NULL;
static int capacity = 200;
module_param(capacity, int, S_IRUGO);
static struct class *cl; 
struct proc_dir_entry *shakespeare_proc;
struct mutex shakespeare_proc_mutex;

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

    for (i = 0; i < count; i++) {
        if (*f_pos >= capacity) *f_pos = 0;
        err = copy_to_user(buf + i, shakespeare_data + *f_pos, 1);
        if (err != 0) {
            mutex_unlock(&shakespeare_mutex);
            return -EFAULT;
        }
        *f_pos += 1;
    }
    mutex_unlock(&shakespeare_mutex);
    return count;
}


ssize_t shakespeare_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    int err, retval = count;
    printk(KERN_INFO THIS "f_pos = %d\n", (int)*f_pos);
    if (mutex_lock_interruptible(&shakespeare_mutex) != 0) {
        return -ERESTARTSYS;
    }
    if (*f_pos >= capacity) {
        mutex_unlock(&shakespeare_mutex);
        printk(KERN_INFO THIS "End of file reached\n");
        return retval;
    }
    if (count > capacity - *f_pos) {
        count = capacity - *f_pos;
        retval = -ENOMEM;
    }

    err = copy_from_user(shakespeare_data + *f_pos, buf, count);
    if (err != 0) {
        mutex_unlock(&shakespeare_mutex);
        return -EFAULT;
    }
    (*f_pos) += count;
    mutex_unlock(&shakespeare_mutex);
    return retval;
}


loff_t shakespeare_llseek (struct file *filp, loff_t off, int whence) {
    loff_t newpos;
    switch(whence) {
        case SEEK_SET:
            newpos = off;
            break;
        case SEEK_CUR:
            newpos = filp->f_pos + off;
            break;
        case SEEK_END:
            newpos = capacity - 1 + off;
            break;
        default: 
            return -EINVAL;
    }
    if (newpos < 0 || newpos >= capacity)
        return -ENOMEM;
    filp->f_pos = newpos;
    return newpos;

}

 

long shakespeare_ioctl (struct file *filp, unsigned int cmd, unsigned long arg) {
    int i;
    signed char increment_value = (signed char) arg;
    switch (cmd) {
        case SHAKESPEARE_INC:
            for (i = 0; i < capacity; i++)
                shakespeare_data[i] += increment_value;
            break;
        default:
            return -ENOTTY; // inappropriate ioctl for device
    }
    return 0;
}

struct file_operations shakespeare_fops = {
    .owner = THIS_MODULE,
    .open = shakespeare_open,
    .release = shakespeare_release,
    .read = shakespeare_read,
    .write = shakespeare_write,
    .llseek = shakespeare_llseek,
    .unlocked_ioctl = shakespeare_ioctl,
};

int shakespeare_proc_open(struct inode *inode, struct file *filp) {
    printk(KERN_DEBUG THIS "Lo! Unveiling proc-apparatus.\n");
    return 0;
}


int shakespeare_proc_release(struct inode *inode, struct file *filp) {
    printk(KERN_DEBUG THIS "Hark! Concluding proc-apparatus.\n");
    return 0;
}

ssize_t shakespeare_proc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int info_size, err;
    char info[255];
    info_size = sprintf(info, "Capacity: %d\n", capacity);
    err = copy_to_user(buf, info, info_size);
    if (err != 0) return -EINVAL; 
    return count;
}


static const struct proc_ops shakespeare_proc_ops = {
    .proc_open  = shakespeare_proc_open,
    .proc_release  = shakespeare_proc_release,
    .proc_read  = shakespeare_proc_read,
};

static void shakespeare_fill_data(void) {
    int i = 0;
    const char poem[] = 
        "Shall I compare thee to a summer's day?\n"
        "Thou art more lovely and more temperate:\n"
        "Rough winds do shake the darling buds of May,\n"
        "And summer's lease hath all too short a date\n";
    const int poem_size = sizeof(poem)/sizeof(char);
    for (i = 0; i < capacity; i++) {
        if (i < poem_size)
            shakespeare_data[i] = poem[i];
        else
            shakespeare_data[i] = 0;
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
    mutex_init(&shakespeare_proc_mutex);
    cdev_init(&shakespeare_cdev, &shakespeare_fops);
    err = cdev_add(&shakespeare_cdev, devno, shakespeare_count);
    if (err != 0) {
        printk(KERN_NOTICE THIS "Mishap %d encountered in apparatus addition.\n", err);
    }

    shakespeare_data = kmalloc(sizeof(char)*capacity, GFP_KERNEL);
    if (shakespeare_data == NULL)
        return -ENOMEM;
    shakespeare_fill_data();

    cl = class_create(THIS_MODULE, "shakespeare");
    device_create(cl, NULL, devno, NULL, "shakespeare");
    shakespeare_proc = proc_create("shakespeare", 0777, NULL, &shakespeare_proc_ops);

    printk(KERN_INFO THIS "Greetings, thou wide world.\n");
    printk(KERN_INFO THIS "Major: %d, Minor: %d\n", shakespeare_major, shakespeare_minor);
    printk(KERN_INFO THIS "Capacity: %d\n", capacity);
    printk(KERN_INFO THIS "SHAKESPEARE_INC number: %lu\n", SHAKESPEARE_INC);
    return 0;
}

static void shakespeare_exit(void) {
    dev_t devno = 0;
    cdev_del(&shakespeare_cdev);
    devno = MKDEV(shakespeare_major, shakespeare_minor);
    device_destroy(cl, devno);
    class_destroy(cl);
    unregister_chrdev_region(devno, shakespeare_count);
    kfree(shakespeare_data);
    printk(KERN_INFO THIS "God save your majesty\n");
}

module_init(shakespeare_init);
module_exit(shakespeare_exit);
