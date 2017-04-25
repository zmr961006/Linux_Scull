/*************************************************************************
	> File Name: irq_qu.c
	> Author: 
	> Mail: 
	> Created Time: 2017年04月25日 星期二 09时18分44秒
 ************************************************************************/
#include<linux/init.h>  
#include<linux/module.h>  
#include<linux/kernel.h>  
#include<linux/interrupt.h>  
   
MODULE_LICENSE("GPL");  

static int irq = 27;  
const char *interface = "my_irq";  
static irqreturn_t myirq_handler(int irq,void *dev);  
   
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
    
    return 0;  

}  
static void  __exit myirq_exit(void){  
    
    
    free_irq(irq,&irq);  
    printk("%s interrupt free %d IRQ\n",interface,irq);  
   
}

module_init(myirq_init);  
module_exit(myirq_exit);  
