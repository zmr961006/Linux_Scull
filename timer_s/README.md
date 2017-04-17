## 内核时间，延时与缓存

## 时间

内核通过定时器中断来跟踪时间流，记录每次时钟周期的滴答数，现代大多数默认时1000HZ ，每次开机后内核会初始化时间变量，然后每过一个时钟周期，变量加一，我们应当充分的信任内核不要随便去改动，除非有特殊的理由。

jiffies 变量用来存储时钟中断次数，是一个64位的无符号长整形。它的访问一直是原子的。

内核定义：

```
extern u64 __jiffy_data jiffies_64;
extern unsigned long volatile __jiffy_data jiffies;

```

比较当前值和缓存值的几个宏：

```
#define time_after(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((b) - (a)) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((a) - (b)) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

```
时钟中断计时法和现实时间的转化。

```
static __always_inline unsigned long usecs_to_jiffies(const unsigned int u);
extern unsigned long timespec64_to_jiffies(const struct timespec64 *value);
extern void jiffies_to_timespec64(const unsigned long jiffies,truct timespec64 *value);
static inline unsigned long timespec_to_jiffies(const struct timespec *value);
static inline void jiffies_to_timespec(const unsigned long jiffies,struct timespec *value);
extern unsigned long timeval_to_jiffies(const struct timeval *value);
extern void jiffies_to_timeval(const unsigned long jiffies, struct timeval *value);
extern clock_t jiffies_to_clock_t(unsigned long x);
extern unsigned long clock_t_to_jiffies(unsigned long x);
```

使用64位寄存器直接获取变量。
static inline u64 get_jiffies_64(void)
{
	return (u64)jiffies;
}



内核就是通过这个全局的变量来获取时间的，一般来说驱动程序是不需要知道墙上时间的，真实世界时间一般放在用户空间使用。

在CODE代码中有一个获取时间模块，我们加载后，可以从/proc/currentime中获取当前时间 
 
结果如下

![d](./image/ss.png)

### 延时

有时我们需要延时比较长的时间，内核给我们提供了一个函数time_before(jiffies,j1) ，这个函数用来不挺的测试jiffies 是否超过了我们设置的j1如果超过了则返回一个负值。
我们使用如下的代码来忙等待我们设置的值。


```
case JIT_BUSY:
			while (time_before(jiffies, j1))
			break;
			
```

我们执行cat 这个命令会不断的调用read ,我们每次查找其中含有22字符的信息，可以发现他是有我们设置的延时的。结果如下：

![ss](./image/sss.png)

此处需要实践运行才能看出来，截图看不出来等待。


一个需要注意的问题是， 如果我们需要等待，最好调用schedule();让出处理器，效果差不多，直接提高效率，此处不再测试。

```
case JIT_SCHED:
			while (time_before(jiffies, j1)) {
				schedule();
			}
			break;
```


####  短延时

当我们需要非常短暂的延时，使用时钟中断显然不是一个好方法，内核给我们提供了一些精度更高的延时函数，这是根据CPU 硬件不同设置不同的。我们不需要关心实现，首先看看定义"linux/delay.h"

```
void calibrate_delay(void);
void msleep(unsigned int msecs);
unsigned long msleep_interruptible(unsigned int msecs);
void usleep_range(unsigned long min, unsigned long max);

static inline void ssleep(unsigned int seconds)
{
	msleep(seconds * 1000);
}
```


### 内核定时器

```
内核定时器是内核用来控制在未来某个时间点（基于jiffies）调度执行某个函数的一种机制，其实现位于 <Linux/timer.h> 和 kernel/timer.c 文件中。
被调度的函数肯定是异步执行的，它类似于一种“软件中断”，而且是处于非进程的上下文中，所以调度函数必须遵守以下规则：
1) 没有 current 指针、不允许访问用户空间。因为没有进程上下文，相关代码和被中断的进程没有任何联系。
2) 不能执行休眠（或可能引起休眠的函数）和调度。
3) 任何被访问的数据结构都应该针对并发访问进行保护，以防止竞争条件。
内核定时器的调度函数运行过一次后就不会再被运行了（相当于自动注销），但可以通过在被调度的函数中重新调度自己来周期运行。
在SMP系统中，调度函数总是在注册它的同一CPU上运行，以尽可能获得缓存的局域性。
内核定时器的数据结构
struct timer_list {
    struct list_head entry;
    unsigned long expires;
    void (*function)(unsigned long);
    unsigned long data;
    struct tvec_base *base;
    /* ... */
};
其中 expires 字段表示期望定时器执行的 jiffies 值，到达该 jiffies 值时，将调用 function 函数，并传递 data 作为参数。
当一个定时器被注册到内核之后，entry 字段用来连接该定时器到一个内核链表中。base 字段是内核内部实现所用的。

需要注意的是 expires 的值是32位的，因为内核定时器并不适用于长的未来时间点。
初始化
在使用 struct timer_list 之前，需要初始化该数据结构，确保所有的字段都被正确地设置。初始化有两种方法。
方法一：
DEFINE_TIMER(timer_name, function_name, expires_value, data);
该宏会定义一个名叫 timer_name 内核定时器，并初始化其 function, expires, name 和 base 字段。
方法二：
struct timer_list mytimer;
setup_timer(&mytimer, (*function)(unsigned long), unsigned long data);
mytimer.expires = jiffies + 5*HZ;

注意，无论用哪种方法初始化，其本质都只是给字段赋值，所以只要在运行 add_timer() 之前，expires, function 
和 data 字段都可以直接再修改。

关于上面这些宏和函数的定义，参见 include/linux/timer.h。
 

定时器要生效，还必须被连接到内核专门的链表中，这可以通过 add_timer(struct timer_list *timer) 来实现。

要修改一个定时器的调度时间，可以通过调用 mod_timer(struct timer_list *timer, unsigned long expires)。
mod_timer() 会重新注册定时器到内核，而不管定时器函数是否被运行过。

注销一个定时器，可以通过 del_timer(struct timer_list *timer) 或 del_timer_sync(struct timer_list *timer)。


del_timer_sync 是用在 SMP 系统上的（在非SMP系统上，它等于del_timer），当要被注销的定时器函数正在另一个 cpu 上运行时，

del_timer_sync() 会等待其运行完，所以这个函数会休眠。

另外还应避免它和被调度的函数争用同一个锁。对于一个已经被运行过且没有重新注册自己的定时器而言，注销函数其实也没什么事可做。

int timer_pending(const struct timer_list *timer)

这个函数用来判断一个定时器是否被添加到了内核链表中以等待被调度运行。注意，当一个定时器函数即将要被运行前，
内核会把相应的定时器从内核链表中删除（相当于注销）
 
```

动态内核定时器机制的原理

Linux内核2.4版中去掉了老版本内核中的静态定时器机制，而只留下动态定时器。相应地在timer_bh()函数中也不再通过run_old_timers()函数来运行老式的静态定时器。动态定时器与静态定时器这二个概念是相对于Linux内核定时器机制的可扩展功能而言的，动态定时器是指内核的定时器队列是可以动态变化的，然而就定时器本身而言，二者并无本质的区别。考虑到静态定时器机制的能力有限，因此Linux内核2.4版中完全去掉了以前的静态定时器机制。 
Linux是怎样为其内核定时器机制提供动态扩展能力的呢？其关键就在于“定时器向量”的概念。所谓“定时器向量”就是指这样一条双向循环定时器队列（对列中的每一个元素都是一个timer_list结构）：对列中的所有定时器都在同一个时刻到期，也即对列中的每一个timer_list结构都具有相同的expires值。显然，可以用一个timer_list结构类型的指针来表示一个定时器向量。 
显然，定时器expires成员的值与jiffies变量的差值决定了一个定时器将在多长时间后到期。在32位系统中，这个时间差值的最大值应该是0xffffffff。因此如果是基于“定时器向量”基本定义，内核将至少要维护0xffffffff个timer_list结构类型的指针，这显然是不现实的。 
另一方面，从内核本身这个角度看，它所关心的定时器显然不是那些已经过期而被执行过的定时器（这些定时器完全可以被丢弃），也不是那些要经过很长时间才会到期的定时器，而是那些当前已经到期或者马上就要到期的定时器（注意！时间间隔是以滴答次数为计数单位的）。 
基于上述考虑，并假定一个定时器要经过interval个时钟滴答后才到期（interval＝expires－jiffies），则Linux采用了下列思想来实现其动态内核定时器机制：对于那些0≤interval≤255的定时器，Linux严格按照定时器向量的基本语义来组织这些定时器，也即Linux内核最关心那些在接下来的255个时钟节拍内就要到期的定时器，因此将它们按照各自不同的expires值组织成256个定时器向量。而对于那些256≤interval≤0xffffffff的定时器，由于他们离到期还有一段时间，因此内核并不关心他们，而是将它们以一种扩展的定时器向量语义（或称为“松散的定时器向量语义”）进行组织。所谓“松散的定时器向量语义”就是指：各定时器的expires值可以互不相同的一个定时器队列。 


