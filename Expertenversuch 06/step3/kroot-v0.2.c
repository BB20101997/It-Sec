#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

typedef long open_call (const char __user*,int,mode_t);

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


static open_call *fopen  = NULL;

asmlinkage long replace_open(const char __user *pathname,int flags , mode_t mode){

	printk(KERN_ALERT "File Opend! %s\n", pathname);

	return (*fopen)(pathname,flags,mode);	

}


static int __init minit (void)
{
	unsigned long *sct = (unsigned long*)aquire_sys_call_table();
	unsigned long *sct_open;

	
	if(sct != NULL){


		sct_open = sct + __NR_open;
		fopen = (open_call *) sct_open;

		printk(KERN_DEBUG "Current Syscall %lx\n", *sct_open);
		
		printk(KERN_DEBUG "Replacing Syscall");

		disable_page_protection();

		fopen =(open_call *)xchg(sct_open,(long)replace_open);	

		enable_page_protection();

		printk(KERN_DEBUG "Current Syscall %lx\n", *sct_open);
		printk(KERN_DEBUG "Backup Syscall  %px\n", fopen);

	}

	printk (KERN_ALERT "Loaded!\n");
	return 0;
}

static void mexit (void)
{

	unsigned long *sct = (unsigned long*)aquire_sys_call_table();
	unsigned long *sct_open; 

	if(sct != NULL && fopen != NULL){

		sct_open = sct+__NR_open;

		printk(KERN_DEBUG "Current Syscall %lx\n", *sct_open);
		printk(KERN_DEBUG "Backup Syscall %px\n", fopen);
		
		printk(KERN_ALERT "Restoring Syscall");
		
		disable_page_protection();

		xchg(sct_open,(long)fopen);

		enable_page_protection();
		
		fopen = NULL;
	
		printk(KERN_DEBUG "Current Syscall %lx\n", *sct_open);
		
	}


	printk (KERN_ALERT "Removed!\n");
}

module_init(minit);
module_exit(mexit);

