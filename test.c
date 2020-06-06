#include<linux/module.h>
#include<linux/init.h>
#include<linux/io.h>
#include<linux/kernel.h>
#include<linux/sched.h>

#define RAM_TEST_BASE 0xa0000
#define RAM_TEST_SIZE 0x10000

volatile unsigned char *vaddr = NULL;

static int __init my_init(void)
{ 
int i=0;

printk(KERN_INFO"iomem loaded..\n");

if(request_mem_region(RAM_TEST_BASE,RAM_TEST_SIZE,"ramtest") < 0) {
	printk("Mem region failed..\n");
}

vaddr = ioremap(RAM_TEST_BASE,RAM_TEST_SIZE);
if(!vaddr) {
	printk(KERN_INFO"Failed to map address..\n");
	release_mem_region(RAM_TEST_BASE,RAM_TEST_SIZE);
}

for(i=0;i<0x100;i+=4) {

	*(vaddr+i) 	= 0xde;
	*(vaddr+i+1)	= 0xad;
	*(vaddr+i+2)	= 0xbe;
	*(vaddr+i+3)	= 0xef;
}

for(i=0;i<0x100;i+=4) {
printk("%x %x\n",(vaddr+i),*(unsigned int *)(vaddr +i));
return 0;
}
}

static void __exit my_exit(void) {

	printk("memory unloaded...\n");
	iounmap((void *)vaddr);
	release_mem_region(RAM_TEST_BASE,RAM_TEST_SIZE);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("RAJAT BANDHA");
MODULE_LICENSE("GPL");

