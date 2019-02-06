#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/fdtable.h>
#include <linux/namei.h>
#include <linux/string.h>

MODULE_LICENSE ("GPL");

//###################################
//#                                 #
//#  COPY FUNCTIONS FROM LMS START  #
//#                                 #
//###################################

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

//#################################
//#                               #
//#  COPY FUNCTIONS FROM LMS END  #
//#                               #
//#################################

//so we don't have to type out the type of the open syscal so often
typedef long open_call (const char __user*,int,umode_t);

typedef long openat_call(int , const char __user* ,int , umode_t);

asmlinkage open_call *fopen  = NULL;
asmlinkage openat_call *fopenat = NULL;

asmlinkage void log_open(long fd){
	//check for valid file discriptor, open may have failed
	if(fd>=0){
		struct file *file = fcheck(fd);
		char buf[256] = {};
		if(file){
			char* ret = d_path(&file->f_path,buf,255);
			if(!IS_ERR(ret)&&strcmp(ret,"/tmp/test.txt")==0){
				printk(KERN_ALERT "File Opend! %s\n", ret);	
			}else{
				//buffer not large enought for path
				//will not be the file we search for anyways
			}
		}		

	}

}

//replacement open syscall 
asmlinkage long replace_open(const char __user *pathname,int flags ,umode_t mode){

	long fd =(*fopen)(pathname,flags,mode);	 

	log_open(fd);

	return fd;
}

//replacement openat syscall 
asmlinkage long replace_openat(int dfd,const char __user *pathname,int flags ,umode_t mode){

	long fd = (*fopenat)(dfd,pathname,flags,mode);	

	log_open(fd);

	return fd;

}

static int __init minit (void)
{
	//create all variables at the beginning of the function to avoid compiler warnings
	unsigned long **sct = aquire_sys_call_table();
	unsigned long **sct_open;
	unsigned long **sct_openat;

	//we should may be consider to fail init if sct is NULL, 
	//we do nothing if it is so its's not harmfull if not to fail
	if(sct != NULL){

		//print syscall table
		printk(KERN_DEBUG "Syscall Table at %px\n", sct);
		
		// point to the address of the open syscall
		sct_open = sct+__NR_open;
		sct_openat = sct+__NR_openat;

		// get the open syscall address		
		// had problems with files beeing opend between the xchg and the returned value beeing stored
		fopen = (open_call *) *sct_open;
		fopenat = (openat_call *) *sct_openat;

		printk(KERN_DEBUG "Current Syscall %px\n", *sct_open);

		printk(KERN_DEBUG "Replacing Syscall");

		disable_page_protection();
	
		//exchange original syscall with our wrapper
		fopen = (open_call *)xchg((long*)sct_open,(long)replace_open);	
		fopenat = (openat_call *)xchg((long*)sct_openat,(long)replace_openat);	

		enable_page_protection();

		printk(KERN_DEBUG "Current Syscall %px\n", *sct_open);
		printk(KERN_DEBUG "Backup Syscall  %px\n", fopen);

	}

	printk (KERN_ALERT "Loaded!\n");
	return 0;
}

static void mexit (void)
{

	//create all variables at the beginning of the function to avoid compiler warnings
	unsigned long **sct = aquire_sys_call_table();
	unsigned long **sct_open;
	unsigned long **sct_openat; 
	
	//we should probably do something if we found the sct in init
	//can we even fail exit? don't know how we could recover in that case
	if(sct != NULL){

		// point to the address of the open syscall
		sct_open = sct+__NR_open;
		sct_openat = sct+__NR_openat;

		printk(KERN_DEBUG "Current Syscall %px\n", *sct_open);
		printk(KERN_DEBUG "Backup Syscall %px\n", fopen);

		printk(KERN_ALERT "Restoring Syscall");

		disable_page_protection();

		//exchange our wrapper syscall with the original	
		if(fopen!=NULL)
			xchg((long*)sct_open,(long)fopen);
		if(fopenat!=NULL)
			xchg((long*)sct_openat,(long)fopenat);

		enable_page_protection();

		//reset fopen to no longer contain the original syscall
		fopen = NULL;
		fopenat = NULL;

		printk(KERN_DEBUG "Current Syscall %px\n", *sct_open);

	}


	printk (KERN_ALERT "Removed!\n");
}

module_init(minit);
module_exit(mexit);

