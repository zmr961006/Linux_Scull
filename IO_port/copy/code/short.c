/*************************************************************************
	> File Name: short.c
	> Author: 
	> Mail: 
	> Created Time: 2017年04月19日 星期三 01时07分49秒
 ************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/delay.h>	/* udelay */
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include <asm/io.h>

#define SHORT_NR_PORTS	8	/* use 8 ports by default */
/*
 * all of the parameters have no "short_" prefix, to save typing when
 * specifying them at load time
 */
static int major = 0;	/* dynamic by default */
module_param(major, int, 0);

static int use_mem = 0;	/* default is I/O-mapped */
module_param(use_mem, int, 0);

/* default is the first printer port on PC's. "short_base" is there too
   because it's what we want to use in the code */
static unsigned long base = 0x378;
unsigned long short_base = 0;
module_param(base, long, 0);

/* The interrupt line is undefined by default. "short_irq" is as above */
static int irq = -1;
volatile int short_irq = -1;
module_param(irq, int, 0);

static int probe = 0;	/* select at load time how to probe irq line */
module_param(probe, int, 0);

static int wq = 0;	/* select at load time whether a workqueue is used */
module_param(wq, int, 0);

static int tasklet = 0;	/* select whether a tasklet is used */
module_param(tasklet, int, 0);

static int share = 0;	/* select at load time whether install a shared irq */
module_param(share, int, 0);


MODULE_LICENSE("Dual BSD/GPL");



int short_open (struct inode *inode, struct file *filp)
{
	
	return 0;
}


int short_release (struct inode *inode, struct file *filp)
{
	return 0;
}


/* first, the port-oriented device */

enum short_modes {SHORT_DEFAULT=0, SHORT_PAUSE, SHORT_STRING, SHORT_MEMORY};

ssize_t do_short_read (struct inode *inode, struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval = count, minor = iminor (inode);
	unsigned long port = short_base + (minor&0x0f);
	void *address = (void *) short_base + (minor&0x0f);
	int mode = (minor&0x70) >> 4;
	unsigned char *kbuf = kmalloc(count, GFP_KERNEL), *ptr;
    
	if (!kbuf)
		return -ENOMEM;
	ptr = kbuf;

	if (use_mem)
		mode = SHORT_MEMORY;
	
	switch(mode) {
	    case SHORT_STRING:
		insb(port, ptr, count);
		rmb();
		break;

	    case SHORT_DEFAULT:
		while (count--) {
			*(ptr++) = inb(port);
			rmb();
		}
		break;

	    case SHORT_MEMORY:
		while (count--) {
			*ptr++ = ioread8(address);
			rmb();
		}
		break;
	    case SHORT_PAUSE:
		while (count--) {
			*(ptr++) = inb_p(port);
			rmb();
		}
		break;

	    default: /* no more modes defined by now */
		retval = -EINVAL;
		break;
	}
	if ((retval > 0) && copy_to_user(buf, kbuf, retval))
		retval = -EFAULT;
	kfree(kbuf);
	return retval;
}


/*
 * Version-specific methods for the fops structure.  FIXME don't need anymore.
 */
ssize_t short_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return do_short_read(filp->f_path.dentry->d_inode, filp, buf, count, f_pos);
}



ssize_t do_short_write (struct inode *inode, struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval = count, minor = iminor(inode);
	unsigned long port = short_base + (minor&0x0f);
	void *address = (void *) short_base + (minor&0x0f);
	int mode = (minor&0x70) >> 4;
	unsigned char *kbuf = kmalloc(count, GFP_KERNEL), *ptr;

	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;
	ptr = kbuf;

	if (use_mem)
		mode = SHORT_MEMORY;

	switch(mode) {
	case SHORT_PAUSE:
		while (count--) {
			outb_p(*(ptr++), port);
			wmb();
		}
		break;

	case SHORT_STRING:
		outsb(port, ptr, count);
		wmb();
		break;

	case SHORT_DEFAULT:
		while (count--) {
			outb(*(ptr++), port);
			wmb();
		}
		break;

	case SHORT_MEMORY:
		while (count--) {
			iowrite8(*ptr++, address);
			wmb();
		}
		break;

	default: /* no more modes defined by now */
		retval = -EINVAL;
		break;
	}
	kfree(kbuf);
	return retval;
}


ssize_t short_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	return do_short_write(filp->f_path.dentry->d_inode, filp, buf, count, f_pos);
}




unsigned int short_poll(struct file *filp, poll_table *wait)
{
	return POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
}






struct file_operations short_fops = {
	.owner	 = THIS_MODULE,
	.read	 = short_read,
	.write	 = short_write,
	.poll	 = short_poll,
	.open	 = short_open,
	.release = short_release,
};


int short_init(void)
{
	int result;

	short_base = base;
	short_irq = irq;

	/* Get our needed resources. */
	if (!use_mem) {
		if (! request_region(short_base, SHORT_NR_PORTS, "short")) {
			printk(KERN_INFO "short: can't get I/O port address 0x%lx\n",
					short_base);
			return -ENODEV;
		}

	} else {
		if (! request_mem_region(short_base, SHORT_NR_PORTS, "short")) {
			printk(KERN_INFO "short: can't get I/O mem address 0x%lx\n",
					short_base);
			return -ENODEV;
		}

		/* also, ioremap it */
		short_base = (unsigned long) ioremap(short_base, SHORT_NR_PORTS);
		/* Hmm... we should check the return value */
	}
	/* Here we register our device - should not fail thereafter */
	result = register_chrdev(major, "short", &short_fops);
	if (result < 0) {
		printk(KERN_INFO "short: can't get major number\n");
		release_region(short_base,SHORT_NR_PORTS);  /* FIXME - use-mem case? */
		return result;
	}
	
	
    printk("the major :%d\n",result);
	


	return 0;
}

void short_cleanup(void)
{
	
}
module_init(short_init);
module_exit(short_cleanup);

