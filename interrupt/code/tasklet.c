/*************************************************************************
	> File Name: tasklet.c
	> Author: 
	> Mail: 
	> Created Time: 2017年04月26日 星期三 08时20分14秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>


static int irq = 27;  
const char *interface = "my_irq";  
static irqreturn_t myirq_handler(int irq,void *dev);  
   

static void tasklet_func(unsigned long value)

{
	int i  = 0;
	for(i = 0;i < 100 ;i++){
	 	mdelay(100);
		printk("dely : %d \n",i);
	}

}

DECLARE_TASKLET(my_tasklet,&tasklet_func,0);
static int __init myirq_init(void)  {  
 
        if(request_irq(irq,myirq_handler,IRQF_SHARED,interface,&irq)){  
            printk(KERN_ERR "%s interrrupt can't register %d IRQ \n",interface,irq);  
            return -EIO;  
        }  
        printk("%s request %d IRQ\n",interface,irq);  
        return 0;  
}  


static irqreturn_t myirq_handler(int irq,void *dev){  
    
    
    printk("my_handle %d IRQ is working\n",irq);  
    
    tasklet_schedule(&my_tasklet);
    
    return 0;  

}  



static void  __exit myirq_exit(void){  
    
    
    free_irq(irq,&irq);  
    printk("%s interrupt free %d IRQ\n",interface,irq);  
   
}



 



module_init(myirq_init);

module_exit(myirq_exit);
