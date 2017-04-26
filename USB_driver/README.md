## USB 驱动程序

### USB的一般化定义
（概念来源于互联网资料）

从1994年11月11日发表了USB V0.7版本以后，USB版本经历了多年的发展，已经发展为3.1版本，成为二十一世纪电脑中的标准扩展接口。当前（2016年）主板中主要是采用USB2.0和USB3.0接口，各USB版本间能很好的兼容。USB用一个4针（USB3.0标准为9针）插头作为标准插头，采用菊花链形式可以把所有的外设连接起来，最多可以连接127个外部设备，并且不会损失带宽。USB需要主机硬件、操作系统和外设三个方面的支持才能工作。二十一世纪的主板一般都采用支持USB功能的控制芯片组，主板上也安装有USB接口插座，而且除了背板的插座之外，主板上还预留有USB插针，可以通过连线接到机箱前面作为前置USB接口以方便使用（注意，在接线时要仔细阅读主板说明书并按图连接，千万不可接错而使设备损坏）。而且USB接口还可以通过专门的USB连机线实现双机互连，并可以通过Hub扩展出更多的接口。USB具有传输速度快，使用方便，支持热插拔，连接灵活，独立供电等优点，可以连接鼠标、键盘、打印机、扫描仪、摄像头、充电器、闪存盘、MP3机、手机、数码相机、移动硬盘、外置光驱/软驱、USB网卡、ADSL Modem、Cable Modem等，几乎所有的外部设备。
理论上USB接口可用于连接多达127个外设，如鼠标、调制解调器和键盘等。USB自从1996年推出后，已成功替代串口和并口，并成为二十一世纪个人电脑和大量智能设备的必配的接口之一。


### 软件结构
每个USB只有一个主机，它包括以下几层：
总线接口
USB总线接口处理电气层与协议层的互连。从互连的角度来看，相似的总线接口由设备及主机同时给出，例如串行接口机（SIE）。USB总线接口由主控制器实现。
USB系统用主控制器管理主机与USB设备间的数据传输。它与主控制器间的接口依赖于主控制器的硬件定义。同时，USB系统也负责管理USB资源，例如带宽和总线能量，这使客户访问USB成为可能。

USB系统还有三个基本组件：

1.主控制器驱动程序（HCD）这可把不同主控制器设备映射到USB系统中。HCD与USB之间的接口叫HCDI，特定的HCDI由支持不同主控制器的操作系统定义，通用主控制器驱动器（UHCD）处于软结构的最底层，由它来管理和控制主控制器。UHCD实现了与USB主控制器通信和控制。

2.USB主控制器，并且它对系统软件的其他部分是隐蔽的。系统软件中的最高层通过UHCD的软件接口与主控制器通信。

3.USB驱动程序（USBD）它在UHCD驱动器之上，它提供驱动器级的接口，满足现有设备驱动器设计的要求。USBD以I/O请求包（IRPs）的形式提供数据传输架构，它由通过特定管道（Pipe）传输数据的需求组成。此外，USBD使客户端出现设备的一个抽象，以便于抽象和管理。作为抽象的一部分，USBD拥有缺省的管道。通过它可以访问所有的USB设备以进行标准的USB控制。该缺省管道描述了一条USBD和USB设备间通信的逻辑通道。

主机软件

在某些操作系统中，没有提供USB系统软件。这些软件本来是用于向设备驱动程序提供配置信息和装载结构的。在这些操作系统中，设备驱动程序将应用提供的接口而不是直接访问USBDI（USB驱动程序接口）结构。

USB客户软件

它是位于软件结构的最高层，负责处理特定USB设备驱动器。客户程序层描述所有直接作用于设备的软件入口。当设备被系统检测到后，这些客户程序将直接作用于外围硬件。这个共享的特性将USB系统软件置于客户和它的设备之间，这就要根据USBD在客户端形成的设备映像由客户程序对它进行处理。

主机各层有以下功能：

1.检测连接和移去的USB设备。

2.管理主机和USB设备间的数据流。

3.连接USB状态和活动统计。

4.控制主控制器和USB设备间的电气接口，包括限量能量供应。

5.HCD提供了主控制器的抽象和通过USB传输的数据的主控制器视角的一个抽象。USBD提供了USB设备的抽象和USBD客户与USB功能间数据
6.传输的一个抽象。USB系统促进客户和功能间的数据传输，并作为USB设备的规范接口的一个控制点。USB系统提供缓冲区管理能力并允许数据传输同步于客户和功能的需求。
### 硬件结构
USB采用四线电缆，其中两根是用来传送数据的串行通道，另两根为下游（Downstream）设备提供电源，对于任何已经成功连接且相互识别的外设，将以双方设备均能够支持的最高速率传输数据。USB总线会根据外设情况在所兼容的传输模式中自动地由高速向低速动态转换且匹配锁定在合适的速率。

USB是基于令牌的总线。类似于令牌环网络或FDDI基于令牌的总线。USB主控制器广播令牌，总线上设备检测令牌中的地址是否与自身相符，通过接收或发送数据给主机来响应。USB通过支持悬挂/恢复操作来管理USB总线电源。USB系统采用级联星型拓扑，该拓扑由三个基本部分组成：主机（Host），集线器（Hub）和功能设备。


主机，也称为根，根结或根Hub，它做在主板上或作为适配卡安装在计算机上，主机包含有主控制器和根集线器（Root Hub），控制着USB

总线上的数据和控制信息的流动，每个USB系统只能有一个根集线器，它连接在主控制器上，一台计算机可能有多个根集线器。

集线器是USB结构中的特定成分，它提供叫做端口（Port）的点将设备连接到USB总线上，同时检测连接在总线上的设备，并为这些设备提供

电源管理，负责总线的故障检测和恢复。集线可为总线提供能源，亦可为自身提供能源（从外部得到电源）。
功能设备通过端口与总线连接。USB同时可做Hub使用。

### 数据传输
主控制器负责主机和USB设备间数据流的传输。这些传输数据被当作连续的比特流。每个设备提供了一个或多个可以与客户程序通信的接口，每个接口由0个或多个管道组成，它们分别独立地在客户程序和设备的特定终端间传输数据。USBD为主机软件的现实需求建立了接口和管道，当提出配置请求时，主控制器根据主机软件提供的参数提供服务。
USB支持四种基本的数据传输模式：控制传输，等时传输，中断传输及数据块传输。每种传输模式应用到具有相同名字的终端，则具有不同的性质。
控制传输类型
支持外设与主机之间的控制，状态，配置等信息的传输，为外设与主机之间提供一个控制通道。每种外设都支持控制传输类型，这样主机与外设之间就可以传送配置和命令/状态信息。
等时（lsochronous）传输类型（或称同步传输）
支持有周期性，有限的时延和带宽且数据传输速率不变的外设与主机间的数据传输。该类型无差错校验，故不能保证正确的数据传输，支持像计算机－电话集成系统（CTI）和音频系统与主机的数据传输。
中断传输类型
支持像游戏手柄，鼠标和键盘等输入设备，这些设备与主机间数据传输量小，无周期性，但对响应时间敏感，要求马上响应。
数据块（Bulk）传输类型
支持打印机，扫描仪，数码相机等外设，这些外设与主机间传输的数据量大，USB在满足带宽的情况下才进行该类型的数据传输。
USB采用分块带宽分配方案，若外设超过当前带宽分配或潜在的要求，则不能进入该设备。同步和中断传输类型的终端保留带宽，并保证数据按一定的速率传送。集中和控制终端按可用的最佳带宽来传输传输数据。


### USB 驱动程序架构

![s](./image/usb.png)

USB 核心为驱动程序提供了一个用于访问控制的统一USB硬件接口。


### USB 端点

USB通信的最基本形式通过端点来，只能向一个方向传送数据，从主机到设备或者从设备到主机。共有4种类型。

控制：控制对于USB不同部分的访问。

中断：每当USB宿主要求传输时，以一个固定的速率传输少量数据。

批量：传输大批量的数据。

等时：传输大批量数据，到达没有保正。

代码结构：

```
/**
 * struct usb_host_endpoint - host-side endpoint descriptor and queue
 * @desc: descriptor for this endpoint, wMaxPacketSize in native byteorder
 * @ss_ep_comp: SuperSpeed companion descriptor for this endpoint
 * @urb_list: urbs queued to this endpoint; maintained by usbcore
 * @hcpriv: for use by HCD; typically holds hardware dma queue head (QH)
 *	with one or more transfer descriptors (TDs) per urb
 * @ep_dev: ep_device for sysfs info
 * @extra: descriptors following this endpoint in the configuration
 * @extralen: how many bytes of "extra" are valid
 * @enabled: URBs may be submitted to this endpoint
 * @streams: number of USB-3 streams allocated on the endpoint
 *
 * USB requests are always queued to a given endpoint, identified by a
 * descriptor within an active interface in a given USB configuration.
 */
struct usb_host_endpoint {   //usb  节点
	struct usb_endpoint_descriptor		desc;
	struct usb_ss_ep_comp_descriptor	ss_ep_comp;
	struct list_head		urb_list;
	void				*hcpriv;
	struct ep_device		*ep_dev;	/* For sysfs info */

	unsigned char *extra;   /* Extra descriptors */
	int extralen;
	int enabled;
	int streams;
};

```

```
/* USB_DT_ENDPOINT: Endpoint descriptor */ usb真正端点信息
struct usb_endpoint_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEndpointAddress;  /*特定端点USB地址*/
	__u8  bmAttributes;      /*端点类型*/
	__le16 wMaxPacketSize;
	__u8  bInterval;

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
	__u8  bRefresh;
	__u8  bSynchAddress;
} __attribute__ ((packed));
```

### 接口

USB端点被捆绑为接口。内核使用struct usb_interface 结构来描述USB接口。

```
/**
 * struct usb_interface - what usb device drivers talk to
 * @altsetting: array of interface structures, one for each alternate
 *	setting that may be selected.  Each one includes a set of
 *	endpoint configurations.  They will be in no particular order.
 * @cur_altsetting: the current altsetting.
 * @num_altsetting: number of altsettings defined.
 * @intf_assoc: interface association descriptor
 * @minor: the minor number assigned to this interface, if this
 *	interface is bound to a driver that uses the USB major number.
 *	If this interface does not use the USB major, this field should
 *	be unused.  The driver should set this value in the probe()
 *	function of the driver, after it has been assigned a minor
 *	number from the USB core by calling usb_register_dev().
 * @condition: binding state of the interface: not bound, binding
 *	(in probe()), bound to a driver, or unbinding (in disconnect())
 * @sysfs_files_created: sysfs attributes exist
 * @ep_devs_created: endpoint child pseudo-devices exist
 * @unregistering: flag set when the interface is being unregistered
 * @needs_remote_wakeup: flag set when the driver requires remote-wakeup
 *	capability during autosuspend.
 * @needs_altsetting0: flag set when a set-interface request for altsetting 0
 *	has been deferred.
 * @needs_binding: flag set when the driver should be re-probed or unbound
 *	following a reset or suspend operation it doesn't support.
 * @authorized: This allows to (de)authorize individual interfaces instead
 *	a whole device in contrast to the device authorization.
 * @dev: driver model's view of this device
 * @usb_dev: if an interface is bound to the USB major, this will point
 *	to the sysfs representation for that device.
 * @pm_usage_cnt: PM usage counter for this interface
 * @reset_ws: Used for scheduling resets from atomic context.
 * @resetting_device: USB core reset the device, so use alt setting 0 as
 *	current; needs bandwidth alloc after reset.
 *
 * USB device drivers attach to interfaces on a physical device.  Each
 * interface encapsulates a single high level function, such as feeding
 * an audio stream to a speaker or reporting a change in a volume control.
 * Many USB devices only have one interface.  The protocol used to talk to
 * an interface's endpoints can be defined in a usb "class" specification,
 * or by a product's vendor.  The (default) control endpoint is part of
 * every interface, but is never listed among the interface's descriptors.
 *
 * The driver that is bound to the interface can use standard driver model
 * calls such as dev_get_drvdata() on the dev member of this structure.
 *
 * Each interface may have alternate settings.  The initial configuration
 * of a device sets altsetting 0, but the device driver can change
 * that setting using usb_set_interface().  Alternate settings are often
 * used to control the use of periodic endpoints, such as by having
 * different endpoints use different amounts of reserved USB bandwidth.
 * All standards-conformant USB devices that use isochronous endpoints
 * will use them in non-default settings.
 *
 * The USB specification says that alternate setting numbers must run from
 * 0 to one less than the total number of alternate settings.  But some
 * devices manage to mess this up, and the structures aren't necessarily
 * stored in numerical order anyhow.  Use usb_altnum_to_altsetting() to
 * look up an alternate setting in the altsetting array based on its number.
 */
struct usb_interface {
	/* array of alternate settings for this interface,
	 * stored in no particular order */
	struct usb_host_interface *altsetting;

	struct usb_host_interface *cur_altsetting;	/* the currently
					 * active alternate setting */
	unsigned num_altsetting;	/* number of alternate settings */

	/* If there is an interface association descriptor then it will list
	 * the associated interfaces */
	struct usb_interface_assoc_descriptor *intf_assoc;

	int minor;			/* minor number this interface is
					 * bound to */
	enum usb_interface_condition condition;		/* state of binding */
	unsigned sysfs_files_created:1;	/* the sysfs attributes exist */
	unsigned ep_devs_created:1;	/* endpoint "devices" exist */
	unsigned unregistering:1;	/* unregistration is in progress */
	unsigned needs_remote_wakeup:1;	/* driver requires remote wakeup */
	unsigned needs_altsetting0:1;	/* switch to altsetting 0 is pending */
	unsigned needs_binding:1;	/* needs delayed unbind/rebind */
	unsigned resetting_device:1;	/* true: bandwidth alloc after reset */
	unsigned authorized:1;		/* used for interface authorization */

	struct device dev;		/* interface specific device info */
	struct device *usb_dev;
	atomic_t pm_usage_cnt;		/* usage counter for autosuspend */
	struct work_struct reset_ws;	/* for resets in atomic context */
};
```

### 配置

USB 接口本身被捆绑为配置。

内核使用usb_host_config 描述USB配置，使用struct usb_device 结构体描述整个USB 设备。

### 概述USB

设备通常有一个或多个配置

配置经常具有一个或者多个更多的接口

接口通常具有一个或多个更多配置

接口没有或者具有一个以上的端点 