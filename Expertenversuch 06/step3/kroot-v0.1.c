#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if (sct[__NR_close] == (unsigned long *) sys_close)
			return sct;

		offset += sizeof(void *);
	}

	return NULL;
}

static void enable_page_protection(void)
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if((value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value | 0x00010000));
}

static void disable_page_protection(void)
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if(!(value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value & ~0x00010000));
}


static long (*fopen)(const char __user*,int,mode_t) = NULL;

asmlinkage long replace_open(const char __user *pathname,int flags , mode_t mode){

	printk(KERN_ALERT "File Opend! %s", pathname);

	return (*fopen)(pathname,flags,mode);	

}


static int __init minit (void)
{
	unsigned long **sct = aquire_sys_call_table();
	unsigned long* sct_open;
	long (*fun)(const char __user*,int, mode_t);

	
	if(sct != NULL){


		sct_open = sct[__NR_open];

		fun = &replace_open;

		printk(KERN_DEBUG "Open Syscall %lx", *sct_open);
		printk(KERN_DEBUG "Replacement Function %px", fun);
		
		printk(KERN_DEBUG "Replacing open Syscall");

		disable_page_protection();

		fun = xchg(sct_open,fun);

		enable_page_protection();

		fopen = fun;

		printk(KERN_DEBUG "Open Syscall %lx", *sct_open);
		printk(KERN_DEBUG "Original Function %px", fopen);


		long file = fopen("/home/bennet/test.txt",O_RDONLY,0);
		sys_close(file);

	}

	printk (KERN_ALERT "Loaded!\n");
	return 0;
}

static void mexit (void)
{

	unsigned long **sct = aquire_sys_call_table();
	unsigned long* sct_open; 
	long (*fun)(const char __user*,int, mode_t);

	if(sct != NULL && fopen != NULL){

		sct_open = sct[__NR_open];

		fun = fopen;

		printk(KERN_DEBUG "Open Syscall %lx", *sct_open);
		printk(KERN_DEBUG "Original Function %px", fopen);
		
		printk(KERN_ALERT "Restoring open Syscall");
		
		disable_page_protection();

		xchg(sct_open,fun);

		enable_page_protection();
		
		fopen = NULL;
	
		printk(KERN_DEBUG "Open Syscall %lx", *sct_open);
		
	}


	printk (KERN_ALERT "Removed!\n");
}

module_init(minit);
module_exit(mexit);

