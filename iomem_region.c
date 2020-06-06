#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/io.h>
#define VIDEO_RAM_BASE 0xE0000
#define VIDEO_RAM_SIZE 0x20000


volatile unsigned char *vaddr = NULL;

static int __init my_init(void)
{

	unsigned long i =0;
	printk(KERN_INFO"IOMEM region loaded..\n");
	if(request_mem_region(VIDEO_RAM_BASE,VIDEO_RAM_SIZE,"VideoRAM")<0)
	{
		printk(KERN_INFO"Mem region is failed..\n");
	}

	vaddr = ioremap(VIDEO_RAM_BASE,VIDEO_RAM_SIZE);

	if(!vaddr)
	{
		printk(KERN_INFO"Failed to map the address.\n");
		release_mem_region(VIDEO_RAM_BASE,VIDEO_RAM_SIZE);
		return 1;
	}

	for(i=0;i<0x10;i++)
		printk("%x %x\n",vaddr+i,*(vaddr+i));

	return 0;
}

static void __exit my_exit(void)
{

	if(vaddr)
		iounmap((void*)vaddr);
	release_mem_region(VIDEO_RAM_BASE,VIDEO_RAM_SIZE);
	printk("IOMEM region unloaded...\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("TechoGenius Acadmey");
MODULE_DESCRIPTION("Iomem region");
MODULE_LICENSE("GPL");

