#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

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
//###################################
//#                                 #
//#  COPY FUNCTIONS FROM WWW START  #
//#                                 #
//###################################
/*
void make_rw(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address,&level);
	if(pte->pte&~_PAGE_RW){
		pte->pte |= _PAGE_RW;
	}	
}

void make_ro(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address,&level);
	pte->pte = pte->pte &~_PAGE_RW;
}
*/
//#################################
//#                               #
//#  COPY FUNCTIONS FROM WWW END  #
//#                               #
//#################################

//so we don't have to type out the type of the open syscal so often
typedef long open_call (const char __user*,int,umode_t);

typedef long openat_call(int , const char __user* ,int , umode_t);

asmlinkage open_call *fopen  = NULL;
asmlinkage openat_call *fopenat = NULL;

//replacement open syscall 
asmlinkage long replace_open(const char __user *pathname,int flags ,umode_t mode){

	printk(KERN_ALERT "File Opend! %s\n", pathname);

	return (*fopen)(pathname,flags,mode);	
}

//replacement openat syscall 
asmlinkage long replace_openat(int dfd,const char __user *pathname,int flags ,umode_t mode){

	printk(KERN_ALERT "File Opend! %s\n", pathname);

	return (*fopenat)(dfd,pathname,flags,mode);	
}

static int __init minit (void)
{
	//create all variables at the beginning of the function to avoid compiler warnings
	unsigned long **sct = aquire_sys_call_table();
	unsigned long **sct_open;
	unsigned long **sct_openat;

	
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
	//	make_rw((unsigned long)sct);		
	
		//exchange original syscall with our wrapper
		fopen = (open_call *)xchg((long*)sct_open,(long)replace_open);	
		fopenat = (openat_call *)xchg((long*)sct_openat,(long)replace_openat);	

	//	make_ro((unsigned long)sct);		
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

	if(sct != NULL && fopen != NULL){

		// point to the address of the open syscall
		sct_open = sct+__NR_open;
		sct_openat = sct+__NR_openat;

		printk(KERN_DEBUG "Current Syscall %px\n", *sct_open);
		printk(KERN_DEBUG "Backup Syscall %px\n", fopen);

		printk(KERN_ALERT "Restoring Syscall");

		disable_page_protection();
	//	make_rw((unsigned long)sct);	

		//exchange our wrapper syscall with the original	
		xchg((long*)sct_open,(long)fopen);
		xchg((long*)sct_openat,(long)fopenat);

	//	make_ro((unsigned long)sct);	
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

