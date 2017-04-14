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


struct scull_dev *scull_devices;

struct file_operations scull_fops = {   /*文件操作函数*/
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read   = scull_read,
    .write  = scull_write,
   // .ioctl  = scull_ioctl, 最新内核删掉了这个接口
    .open   = scull_open,
    .release= scull_release,
};
    


/*
 * Here are our sequence iteration methods.  Our "position" is
 * simply the device number.
 */
static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
	if (*pos >= scull_nr_devs)
		return NULL;   /* No more to read */
	return scull_devices + *pos;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= scull_nr_devs)
		return NULL;
	return scull_devices + *pos;
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

/*
 * The proc filesystem: function to read and entry
 */

int scull_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int i, j, len = 0;
	int limit = count - 80; /* Don't print more than this */

	for (i = 0; i < scull_nr_devs && len <= limit; i++) {
		struct scull_dev *d = &scull_devices[i];
		struct scull_qset *qs = d->data;
		if (down_interruptible(&d->sem))
			return -ERESTARTSYS;
		len += sprintf(buf+len,"\nDevice %i: qset %i, q %i, sz %li\n",
				i, d->qset, d->quantum, d->size);
		for (; qs && len <= limit; qs = qs->next) { /* scan the list */
			len += sprintf(buf + len, "  item at %p, qset at %p\n",
					qs, qs->data);
			if (qs->data && !qs->next) /* dump only the last item */
				for (j = 0; j < d->qset; j++) {
					if (qs->data[j])
						len += sprintf(buf + len,
								"    % 4i: %8p\n",
								j, qs->data[j]);
				}
		}
		up(&scull_devices[i].sem);
	}
	*eof = 1;
	return len;
}


/*便利设备链表*/
struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
	struct scull_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
	if (! qs) {
		qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct scull_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct scull_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}





/*安装DEV 结构到这个scull_devices*/
static void scull_setup_cdev(struct scull_dev *dev,int index){

    int err,devno = MKDEV(scull_major,scull_minor + index);
    cdev_init(&dev->cdev,&scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops   = &scull_fops;
    err = cdev_add(&dev->cdev,devno,1);

    if(err){
        printk("error %d adding scull%d",err,index);
    }

}

/*几个函数调用方法*/


int     scull_p_init(dev_t dev){
    return 0;
}
void    scull_p_cleanup(void){
    //return 0;
}
int     scull_access_init(dev_t dev){
    return 0;
}
void    scull_access_cleanup(void){
    //return 0;
}
/*删除设备的空间*/
int scull_trim(struct scull_dev *dev){

    struct scull_qset *next,*dptr;
    int qset = dev->qset;
    int i;
    for(dptr = dev->data;dptr;dptr = next){
        if(dptr->data){
            for(i = 0;i < qset;i++){
                kfree(dptr->data[i]);
            }
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;


}


int scull_open(struct inode* inode,struct file *filp){
	struct scull_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		scull_trim(dev); /* ignore errors */
		up(&dev->sem);
	}
	return 0;          /* success */
  
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos){
	struct scull_dev *dev = filp->private_data; 
	struct scull_qset *dptr;	/* the first listitem */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = scull_follow(dev, item);

	if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

  out:
	up(&dev->sem);
	return retval;
   
}
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos){
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scull_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

        /* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

  out:
	up(&dev->sem);
	return retval;
}
loff_t  scull_llseek(struct file *filp, loff_t off, int whence){
    return 0;
}
int     scull_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg){
    return 0;
}

int scull_release(struct inode * inode,struct file*filp){
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
	/*create_proc_read_entry("scullmem", 0  {default mode},
			NULL { parent dir } , {scull_read_procmem},
			NULL { client data} );
	entry = create_proc_entry("scullseq", 0, NULL);
    */
    proc_create("scullmem",0,NULL,&scull_proc_ops);
    entry = proc_create("scullseq",0,NULL,&scull_proc_ops);

	//if (entry)
	//	entry->proc_fops = &scull_proc_ops;
}

static void scull_remove_proc(void)
{
	/* no problem if it was not registered */
	remove_proc_entry("scullmem", NULL /* parent dir */);
	remove_proc_entry("scullseq", NULL);
}


/*_______________________________________________________________________________________*/


void scull_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(scull_major, scull_minor);

	/* Get rid of our char dev entries */
	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			scull_trim(scull_devices + i);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}
    scull_remove_proc();/*创建测试的proc 文件*/
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, scull_nr_devs);

	/* and call the cleanup functions for friend devices */
	//scull_p_cleanup();
	//scull_access_cleanup();

}






int scull_init_module(void)   /*获取主设备号，或者创建设备编号*/
{
    int result ,i;
    dev_t dev = 0;

    if(scull_major){
        dev = MKDEV(scull_major,scull_minor);     /*将两个设备号转换为dev_t类型*/
        result = register_chrdev_region(dev,scull_nr_devs,"scull");/*申请设备编号*/
    }else{
        result = alloc_chrdev_region(&dev,scull_minor,scull_nr_devs,"scull");/*分配主设备号*/
        scull_major = MAJOR(dev);
    }

    if(result < 0){
        printk("scull : cant get major %d\n",scull_major);
        return result;
    }else{
        printk("make a dev %d %d\n",scull_major,scull_minor);
    }
    /*分配设备的结构体*/
    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev),GFP_KERNEL);

    if(!scull_devices){
        result = -1;
        goto fail;
    }
    memset(scull_devices,0,scull_nr_devs * sizeof(struct scull_dev));

    for(i = 0;i < scull_nr_devs;i++){
        scull_devices[i].quantum = scull_quantum;
        scull_devices[i].qset = scull_qset;
        sema_init(&scull_devices[i].sem,1);
        /*sema_init  是内核用来新代替dev_INIT 的函数，初始化互斥量*/
        scull_setup_cdev(&scull_devices[i],i);
        /*注册每一个设备到总控结构体*/
    }

    dev = MKDEV(scull_major,scull_minor + scull_nr_devs);
    //dev += scull_p_init(dev);
    //dev += scull_access_init(dev);
    scull_create_proc();  /*创建/proc 下文件*/

    return 0;

fail:
    scull_cleanup_module();
    return result;

}





module_init(scull_init_module);
module_exit(scull_cleanup_module);


