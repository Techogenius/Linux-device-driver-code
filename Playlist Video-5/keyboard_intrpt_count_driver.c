#include <linux/kernel.h>
#include <linux/init.h>	  // kernel module programming
#include <linux/module.h> // module_init, module_exit
#include <linux/kdev_t.h> // device files
#include <linux/fs.h>     // file structure
#include <linux/cdev.h>   // cdev structure
#include <linux/device.h>
#include <linux/slab.h>   // kmalloc
#include <linux/uaccess.h>// copy_from_user, copy_to_user

#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <asm/io.h>

// FOR DIFFERENT DEVICES, THERE ARE DIFF NUMBERS :
// 1-KEYBOARD LIKE THAT, SO THIS WILL GET INTERRUPTED BY A KEY PRESS
// TIMER-0.....
#define IRQ_NO 1
unsigned int i = 0 ;

// INTERRUPT HANDLER FOR IRQ 1
static irqreturn_t irq_handler(int irq, void *dev_id) {
	printk(KERN_INFO"--k e y b o a r d: interrupt occured <> %d--\n", i) ;
	i++ ;
	return IRQ_HANDLED ;
}

volatile int chr_value = 0 ;
struct kobject *kobj_ref ;
// ^^NEW

static int  __init chr_driver_init(void) ;
// USE __init chr_driver_init
dev_t dev = 0 ; // alloc_chrdev_region -> cdev_init -> device_create
static struct cdev chr_cdev ; // cdev_init -> cdev_add
// for the cdev_init

// PROTOTYPES FOR READING WRITING DATA
// these have to be defined first, then only you can write the static struct file_operations fops,
// else there will be warnings as definitions of these not found
static int chr_open(struct inode *inode, struct file *file) ;
static int chr_release(struct inode *inode, struct file *file) ;

static ssize_t chr_read(struct file *filp, char __user *buf, size_t len, loff_t *off) ;
static ssize_t chr_write(struct file *filp, const char *buf, size_t len, loff_t *off) ;

static struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = chr_read,
        .write   = chr_write,
        .open    = chr_open,
        .release = chr_release
};

static struct class *dev_class ; // class_create -> device_create
// USE __init chr_driver_init

static void __exit chr_driver_exit(void) ;

// static int chr_open :
#define mem_size 1024
uint8_t *kernel_buffer ;

static int chr_open(struct inode *inode, struct file *file) {
	// allocating physical memory
	if( (kernel_buffer = kmalloc(mem_size, GFP_KERNEL)) == 0 ) {
		printk(KERN_INFO"ERROR >< Cannot allocate the memory to the kernel\n") ;
		return -1 ;
	}

	printk(KERN_INFO"[INFO] SUCCESS <> Device file opened\n") ;
	return 0 ;
}

static int chr_release(struct inode *inode, struct file *file) {
	kfree(kernel_buffer) ; // free that was allocated by open
	printk(KERN_INFO"[INFO] SUCCESS <> Device file closed\n") ;
	return 0 ;
}

// WHENEVER WE WILL CALL read, write FROM USERSPACE, THESE WILL BE CALLED
static ssize_t chr_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	int res=copy_to_user(buf, kernel_buffer, mem_size) ;
	printk(KERN_INFO"[INFO] Data read : DONE <-> return from copy_to_user : %d\n", res ) ;
        return mem_size ;
}

static ssize_t chr_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
	int res=copy_from_user(kernel_buffer, buf, len) ;
	printk(KERN_INFO"[INFO] Data written successfully <-> return from copy_from_user :  %d\n", res ) ;
	return len ;
}

static int __init chr_driver_init(void) {
	// allocating MAJOR number dynamically
	// alloc_chrdev_region(dev_t *dev, unsigned int firstMinor, unsigned int count, char *name)
	if( (alloc_chrdev_region(&dev, 0, 1, "keyboard_intrpt_count_device") ) < 0 ) {
		printk(KERN_INFO"ERROR >< Cannot allocate the MAJOR number\n") ;
		return -1 ;
	}

	printk(KERN_INFO"[INFO] MAJOR : %d <-> minor : %d\n", MAJOR(dev), MINOR(dev) ) ;

	// creating cdev structure
	// fops : file operation structure : to read/write a file
	cdev_init(&chr_cdev, &fops) ;
	// just initialised

	// adding character device to the system
	if( (cdev_add(&chr_cdev, dev, 1)) < 0 ) {
		printk(KERN_INFO"ERROR >< Cannot add the device to the system\n") ;
		// as we cannot add, so we are jumping to the unregister code
		goto r_class ;
	}

	// creating struct class
	if( (dev_class = class_create(THIS_MODULE, "chr_class" )) == NULL ) {
	//                                         ^any name
		printk(KERN_INFO"ERROR >< Cannot create the struct class\n") ;
		// as we cannot create, so just unregister
		goto r_class ;
	}

	// creating device
	// dev_class = output of class_create, from above
	if( (device_create(dev_class, NULL, dev, NULL, "keyboard_intrpt_count_device")) == NULL ) {
		printk(KERN_INFO"ERROR >< Cannot create the device\n") ;
		// class created but could not create device, so destroy the creation
		goto r_device ;
		//   ^notice the label, it is for destroying device not class
	}

	// NEW-------------------------------->IRQ
	if( request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "keyboard_intrpt_count_device", (void *)(irq_handler) ) ) {
		printk(KERN_INFO"ERROR >< keyboard_intrpt_count_device : CANNOT REGISTER IRQ\n") ;
		goto irq ;
		// NEW irq label-------------->IRQ
	}

	printk(KERN_INFO"[INFO] SUCCESS <> Device driver insert done properly\n") ;
	return 0 ;

	irq :
		free_irq( IRQ_NO, (void *)(irq_handler) ) ;

	r_device :
		class_destroy(dev_class) ;

	r_class :
		unregister_chrdev_region(dev, 1) ;
		return -1 ;
}

void __exit chr_driver_exit(void) {
	free_irq(IRQ_NO, (void *)(irq_handler) ) ;
	device_destroy(dev_class, dev) ;
	class_destroy(dev_class) ;
	cdev_del(&chr_cdev) ;
	unregister_chrdev_region(dev, 1) ;
	printk(KERN_INFO"[INFO] SUCCESS <> Device driver is removed successfully\n") ;
}

// to specify our custom name init and exit : chr_driver_init/exit
// so that init and exit can be done
module_init(chr_driver_init) ;
module_exit(chr_driver_exit) ;

// macros, there are many, but we need these to convey the important info
MODULE_LICENSE("GPL") ;
MODULE_AUTHOR("-----------------------") ;
MODULE_DESCRIPTION("The character device driver") ;
