## PCI驱动程序

一．理论

1.      PCI总线的特点：

（1）速度上快，时钟频率提高到33M，而且还为进一步把时钟频率提高到66MHZ、总线带宽提高到64位留下了余地。（2）对于地址的分配和设置，系统软件课自动设置，每块外设通过某种途径告诉系统该外设有几个存储区间和I/O地址区间，每个区间的大小以及本地地址。系统软件知道了总共有多少外设以及各种的存储空间后就会统一为外设分配物理地址。（3）对于总线的竞争，PCI总线上配备了一个仲裁器，遇到冲突，仲裁器会选择其中之一暂时成为当前的主设备，而其他只能等待。同时考虑到这样的效率问题，PCI总线为写提纲了缓冲，故写比读会快。（4）对于总线扩充问题，PCI总线引入HOST-PCI桥（北桥）,PCI-PCI桥,PCI-ISA桥（南桥）。CPU与内存通过系统总线连接，北桥连接内存控制器与主PCI总线，南桥连接主PCI总线和ISA总线，PCI-PCI桥连接主PCI总和次层PCI总线。

2.      PCI设备概述

每个PCI设备有许多地址配置的寄存器，初始化时要通过这些寄存器来配置该设备的总线地址，一旦完成配置以后，CPU就可以访问该设备的各项资源了。PCI标准规定每个设备的配置寄存器组最多可以有256个连续的字节空间，开头64个字节叫头部，分为0型（PCI设备）和1型（PCI桥）头部，头部开头16个字节是设备的类型、型号和厂商等。这些头部寄存器除了地址配置的作用，还能使CPU能够探测到相应设备的存在，这样就不需要用户告诉系统都有哪些设备了，而是改由CPU通过一个号称枚举的过程自动扫描探测所有挂接在PCI总线上的设备。

设备的配置寄存器组采用相同的地址，由所在总线的PCI桥在访问时附加上其他条件区分，对于I386处理器，有两个32位寄存器，0XCF8为地址寄存器，0XCFC为数据寄存器。地址寄存器写入的内容包括总线号，设备号，功能号。逻辑地址（XX:YY.Z），XX表示PCI总线号，最多256个总线。YY表示PCI设备号，最多32个设备。Z表示PCI设备功能号，最多8个功能。

3.      查询PCI总线和设备的命令

     查看PCI总线和PCI设备组成的树状图 lspci –t        

     查看配置区的情况 lspci –x，注意PCI寄存器是小端格式

4.      PCI总线架构

所有的根总线都链接在pci_root_buses链表中。Pci_bus ->device链表链接着该总线下的所有设备。而pci_bus->children链表链接着它的下层总线，对于pci_dev来说，pci_dev->bus指向它所属的pci_bus. Pci_dev->bus_list链接在它所属bus的device链表上。此外，所有pci设备都链接在pci_device链表中。

5.      ********

二．PCI驱动

1.      PCI寻找空间

   PCI设备包括杀个寻址空间：配置空间，I/O端口空间，内存空间。

1.1 PCI配置空间：

内核为驱动提供的函数：

pci_read_config_byte/word/dword(struct pci_dev *pdev, int offset, int *value)

pci_write_config_byte/word/dword(struct pci_dev *pdev, int offset, int *value)

配置空间的偏移定义在include/Linux/pci_regs.h

1.2 PCI的I/O和内存空间：

从配置区相应寄存器得到I/O区域的基址：

pci_resource_start(struct pci_dev *dev,  int bar)    Bar值的范围为0-5。

从配置区相应寄存器得到I/O区域的内存区域长度：

pci_resource_length(struct pci_dev *dev,  int bar)    Bar值的范围为0-5。

从配置区相应寄存器得到I/O区域的内存的相关标志：

pci_resource_flags(struct pci_dev *dev,  int bar)    Bar值的范围为0-5。

申请I/O端口:

request_mem_region(io_base, length, name)

读写：

inb()  inw()  inl()   outb()     outw()  outl()

### 注册一个PCI 设备

```
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>


static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_3), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

static unsigned char skel_get_revision(struct pci_dev *dev)
{
	u8 revision;

	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	return revision;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	/* Do probing type stuff here.  
	 * Like calling request_region();
	 */
	pci_enable_device(dev);
	
	if (skel_get_revision(dev) == 0x42)
		return -ENODEV;


	return 0;
}

static void remove(struct pci_dev *dev)
{
	/* clean up any allocated resources and stuff here.
	 * like call release_region();
	 */
}

static struct pci_driver pci_driver = {
	.name = "pci_skel",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

static int __init pci_skel_init(void)
{
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{
	pci_unregister_driver(&pci_driver);
}

MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);

```

参考：http://blog.csdn.net/weiqing1981127/article/details/8031541