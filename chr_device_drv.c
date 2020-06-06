#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/jiffies.h>
#include<linux/timer.h>

#define TIMEOUT 5000 //millisecond
#define IRQ_NO 1 

unsigned int i =0;

void tasklet_func(unsigned long );

struct tasklet_struct *tasklet;
unsigned long chr_spinlock_variable =0;

static struct timer_list chr_timer;

/* Declare the tasklet */

//DECLARE_TASKLET(tasklet,tasklet_func,1);

DEFINE_SPINLOCK(chr_spinlock);

//timer callback function which is called when timer expires

void timer_callback(struct timer_list *timer)
{
	printk(KERN_INFO"in Timer callback function[%d]\n",i++);

	// Re-enable the timer which will make this as periodic timer */

	mod_timer(&chr_timer,msecs_to_jiffies(TIMEOUT));

}
/* Tasklet function body */

void tasklet_func(unsigned long data)
{
	spin_lock_irq(&chr_spinlock);
	chr_spinlock_variable++;	
//	printk(KERN_INFO" Executing the Tasklet function : data =%lu\n",chr_spinlock_variable);
	spin_unlock_irq(&chr_spinlock); 
}

static irqreturn_t irq_handler(int irq,void *dev_id) {
	spin_lock_irq(&chr_spinlock);
	chr_spinlock_variable++;
  //      printk(KERN_INFO "Interrupt Occurred and executing ISR routine.. %lu\n",chr_spinlock_variable);
	spin_unlock_irq(&chr_spinlock);
	/* Scheduling the tasklet */    
	tasklet_schedule(tasklet);
        return IRQ_HANDLED;
}

dev_t dev = 0;
static struct class *dev_class;
static struct cdev chr_cdev;
struct kobject *kobj_ref;
 
static int __init chr_driver_init(void);
static void __exit chr_driver_exit(void);

static struct task_struct *chr_thread1;
static struct task_struct *chr_thread2;
 
/*************** Driver Fuctions **********************/
static int chr_open(struct inode *inode, struct file *file);
static int chr_release(struct inode *inode, struct file *file);
static ssize_t chr_read(struct file *filp,char __user *buf, size_t len,loff_t * off);
static ssize_t chr_write(struct file *filp,const char *buf, size_t len, loff_t * off); 

int thrd_func1(void *p);
int thrd_func2(void *p);

int thrd_func1(void *p)
{
//int i=0;
	while(!kthread_should_stop()) {
		if(!spin_is_locked(&chr_spinlock)) {
	//printk(KERN_INFO"spinlock is not locked in thread function1..\n");
	}
	spin_lock(&chr_spinlock);
	if(spin_is_locked(&chr_spinlock)) {
	//	printk(KERN_INFO"spinlock is locked in thread function 1..\n");
	}
	chr_spinlock_variable++;
	//printk(KERN_INFO"In thread function 1 %lu\n",chr_spinlock_variable);
	spin_unlock(&chr_spinlock);
	msleep(1000);
	}
return 0;
}

int thrd_func2(void *p)
{
	while(!kthread_should_stop()) {
		if(!spin_is_locked(&chr_spinlock)) {
//	printk(KERN_INFO"spinlock is not locked in thread function2..\n");
	}
	spin_lock(&chr_spinlock);
	if(spin_is_locked(&chr_spinlock)) {
	//	printk(KERN_INFO"spinlock is locked in thread function 2..\n");
	}
	chr_spinlock_variable++;
//	printk(KERN_INFO"In thread function 2 %lu\n",chr_spinlock_variable);
	spin_unlock(&chr_spinlock);
	msleep(1000);
	}
return 0;
}


static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = chr_read,
        .write          = chr_write,
        .open           = chr_open,
        .release        = chr_release,
};
  
static int chr_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Opened...!!!\n");
        return 0;
}
 
static int chr_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Closed...!!!\n");
        return 0;
}
 
static ssize_t chr_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read function\n");
        return 0;
}
static ssize_t chr_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write Function\n");
        return 0;
}
 
 
static int __init chr_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "chr_Dev")) <0){
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&chr_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&chr_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"chr_class")) == NULL){
            printk(KERN_INFO "Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"chr_device")) == NULL){
            printk(KERN_INFO "Cannot create the Device 1\n");
            goto r_device;
        }

        if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "chr_device", (void *)(irq_handler))) {
            printk(KERN_INFO "chr_device: cannot register IRQ ");
                    goto irq;
        }

	//setup your timer to call timer callback function */
	timer_setup(&chr_timer,timer_callback,0);

	//setup the timer interval to base on TIMEOUT Macro
	mod_timer(&chr_timer,jiffies + msecs_to_jiffies(TIMEOUT));

	tasklet = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
	if(tasklet == NULL) {
		printk(KERN_INFO"cannot allocate the memory..\n");
		goto irq;
	}

	
	tasklet_init(tasklet,tasklet_func,0); //Dynamic Method


	chr_thread1 = kthread_create(thrd_func1,NULL,"chr thread 1");
	if(chr_thread1) {
		wake_up_process(chr_thread1);
	}else {
		printk(KERN_INFO"Unable to create the thread.\n");
		goto r_device;
	}


	chr_thread2 = kthread_run(thrd_func2,NULL,"chr thread 2");
	if(chr_thread2) {
		printk(KERN_INFO"Successfully created the kernel thread..\n");
	}else {
		printk(KERN_INFO"unable to create the thread..\n");
		goto r_device;
	}
        printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;
 
irq:
        free_irq(IRQ_NO,(void *)(irq_handler));

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&chr_cdev);
        return -1;
}
 
void __exit chr_driver_exit(void)
{
	kthread_stop(chr_thread1);
	kthread_stop(chr_thread2);
        free_irq(IRQ_NO,(void *)(irq_handler));
	tasklet_kill(tasklet);
	del_timer(&chr_timer);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
	cdev_del(&chr_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(chr_driver_init);
module_exit(chr_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechoGenius Academy");
MODULE_DESCRIPTION("Character Device Driver using Interrupts");
