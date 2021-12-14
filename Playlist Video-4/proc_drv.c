#include <linux/kernel.h>
#include <linux/init.h>	  // kernel module programming
#include <linux/module.h> // module_init, module_exit
#include <linux/kdev_t.h> // device files
#include <linux/fs.h>     // file structure
#include <linux/cdev.h>   // cdev structure
#include <linux/device.h>
#include <linux/slab.h>   // kmalloc
#include <linux/uaccess.h>// copy_from_user, copy_to_user

// NEW FOR PROC FS
#include <linux/proc_fs.h>
char chr_array[64] ;
static int len=1; // This variable we will need for read and write functions

#include <linux/ioctl.h>
// DEFINE THE ioctl CODE
#define WR_DATA _IOW('a', 'a', int32_t*)
//            magic number, command number, type
#define RD_DATA _IOR('a', 'b', int32_t*)
int32_t val = 0 ;
static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg) ; // static struct file_operations fops : .unlocked_ioctl

static int  __init chr_driver_init(void) ;
// USE __init chr_driver_init************************************************************************
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

// FOR PROC FS- - - - - - - ->
static int open_proc(struct inode *inode, struct file *file) ;
static int release_proc(struct inode *inode, struct file *file) ;

static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length, loff_t *offset) ;
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t *off) ;
// for these functions also, we need to create a new file operation structure

// previously -( for the normal character device driver )-
static struct file_operations fops = {
        .owner          = THIS_MODULE,
        .read           = chr_read,
        .write          = chr_write,
        .open           = chr_open,
	.unlocked_ioctl = chr_ioctl, // NEW
        .release        = chr_release
};

// new -( for p r o c f s )-
// https://stackoverflow.com/questions/64931555/how-to-fix-error-passing-argument-4-of-proc-create-from-incompatible-pointer
/* static struct file_operations proc_fops -----> worked for L4.15.0-72
	but will not  work for L5.11.0.40/41
   solution is to replace with --struct proc_ops-- for kernel version 5.6 or later
   change names of the methods too (e.g. change .release to .proc_release */
	struct proc_ops proc_fops = {
        .proc_open    = open_proc,
        .proc_read    = read_proc,
        .proc_write   = write_proc,
        .proc_release = release_proc
};

static struct class *dev_class ; // class_create -> device_create
// USE __init chr_driver_init************************************************************************

static void __exit chr_driver_exit(void) ;

// Defining functions for interacting with the p r o c f s...........................................
static int open_proc(struct inode *inode, struct file *file) {
	printk(KERN_INFO"[INFO] SUCCESS <> procfs file is opened\n") ;
	return 0 ;
}

static int release_proc(struct inode *inode, struct file *file) {
        printk(KERN_INFO"[INFO] SUCCESS <> procfs file is released\n") ;
        return 0 ;
}

static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
	int res ;
	printk(KERN_INFO"[INFO] reading procfs file system\n") ;
	if(len)
		len = 0 ;
	else {
		len = -1 ;
		return 0 ;
	}
	res=copy_to_user(buffer, chr_array, 40) ;
	printk(KERN_INFO"[INFO] copy_to_user returned %d\n", res) ;
	return length ;
}

static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t *off) {
	int res ;
	printk(KERN_INFO"[INFO] writing in procfs file system\n") ;
	res=copy_from_user(chr_array, buff, len) ;
	printk(KERN_INFO"[INFO] copy_from_user returned %d\n", res) ;
	return len ;
}
// Defining functions for interacting with the p r o c f s...........................................

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

// NEW----------------------------------->>
static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int res ;
	switch(cmd) {
		case WR_DATA:
			res=copy_from_user(&val, (int32_t*)arg, sizeof(val)) ;
			printk(KERN_INFO"[INFO] val = %d <-> return from copy_to_user : %d\n", val, res) ;
			break ;
		case RD_DATA:
			res=copy_to_user((int32_t*)arg, &val, sizeof(val)) ;
			printk(KERN_INFO"[INFO] Data read : DONE <-> return from copy_to_user : %d\n", res ) ;
			break ;
	}
	return 0 ;
}

static int __init chr_driver_init(void) {
	// allocating MAJOR number dynamically
	// alloc_chrdev_region(dev_t *dev, unsigned int firstMinor, unsigned int count, char *name)
	if( (alloc_chrdev_region(&dev, 0, 1, "chr_device") ) < 0 ) {
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
	if( (device_create(dev_class, NULL, dev, NULL, "chr_device")) == NULL ) {
		printk(KERN_INFO"ERROR >< Cannot create the device\n") ;
		// class created but could not create device, so destroy the creation
		goto r_device ;
		//   ^notice the label, it is for destroying device not class
	}

	/*------CREATING THE proc ENTRY------*/
	proc_create("chr_proc", 0666, NULL, &proc_fops) ;
	// proc_create(name of the proc_entry, mode, parent, file operation structure
	// parent means it will search a diectory in proc directory, if NULL then /proc will be parent directory

	printk(KERN_INFO"[INFO] SUCCESS <> Device driver insert done properly\n") ;
	return 0 ;

	r_device :
		class_destroy(dev_class) ;

	r_class :
		unregister_chrdev_region(dev, 1) ;
		return -1 ;
}

void __exit chr_driver_exit(void) {
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
MODULE_AUTHOR("----------------------") ;
MODULE_DESCRIPTION("character device driver-ioctl-procfs") ;
