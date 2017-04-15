/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: 2017年03月24日 星期五 11时41分42秒
 ************************************************************************/

//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>  /*创建/proc 文件以用来和用户交互数据*/
#include <linux/cdev.h>
#include <linux/proc_ns.h>  /*proc 在新版中移动到了此目录下*/
//#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include <net/snmp.h>
#include <linux/ipv6.h>
#include <net/if_inet6.h>



#include "scull.h"		/* local definitions */

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset    = SCULL_QSET;





static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
	
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	
}

static void scull_seq_stop(struct seq_file *s, void *v)
{
	/* Actually, there's nothing to do here */
}

static int scull_seq_show(struct seq_file *s, void *v)
{
	struct timeval tv1;
	struct timespec tv2;
	unsigned long j1;
	u64 j2;

	/* get them four */
	j1 = jiffies;
	j2 = get_jiffies_64();
	do_gettimeofday(&tv1);
	tv2 = current_kernel_time();

	/* print */
	//len=0;
	//len +=
 	seq_printf(s,"0x%08lx 0x%016Lx %10i.%06i\n"
		       "%40i.%09i\n",
		       j1, j2,
		       (int) tv1.tv_sec, (int) tv1.tv_usec,
		       (int) tv2.tv_sec, (int) tv2.tv_nsec);
	//*start = buf;
	return 0;
}



/*便利设备链表*/
struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{

	return 0;
}



/*
 * Tie the sequence operators up.
 */
static struct seq_operations scull_seq_ops = {
	.start = scull_seq_start,
	.next  = scull_seq_next,
	.stop  = scull_seq_stop,
	.show  = scull_seq_show
};

/*
 * Now to implement the /proc file we need only make an open
 * method which sets up the sequence operators.
 */
static int scull_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &scull_seq_ops);
}

/*
 * Create a set of file operations for our proc file.
 */
static struct file_operations scull_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = scull_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
	

/*proc 文件的创建与移除*/

static void scull_create_proc(void)
{
	struct proc_dir_entry *entry;

    proc_create("currentime",0,NULL,&scull_proc_ops);

    printk("create mode");
	
}

static void scull_remove_proc(void)
{
	
	remove_proc_entry("currentime", NULL /* parent dir */);
	
}


/*_______________________________________________________________________________________*/


void scull_cleanup_module(void)
{
    int devno;
    scull_remove_proc();/*创建测试的proc 文件*/
	
	unregister_chrdev_region(devno, scull_nr_devs);

	

}






int scull_init_module(void)   /*获取主设备号，或者创建设备编号*/
{
  
    dev_t dev;
    int result;
    if(scull_major){
        dev = MKDEV(scull_major,scull_minor);     
        result = register_chrdev_region(dev,scull_nr_devs,"scull");
    }else{
        result = alloc_chrdev_region(&dev,scull_minor,scull_nr_devs,"scull");
        scull_major = MAJOR(dev);
    }
  
    scull_create_proc(); 

    return 0;


}





module_init(scull_init_module);
module_exit(scull_cleanup_module);


