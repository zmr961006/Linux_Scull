# Concurrency and competition
# 并发与竞态


### 竞态
竞态条件（race condition），从多进程间通信的角度来讲，是指两个或多个进程对共享的数据进行读或写的操作时，最终的结果取决于这些进程的执行顺序。

### 并发
   在操作系统中，并发是指一个时间段中有几个程序都处于已启动运行到运行完毕之间，且这几个程序都是在同一个处理机上运行，但任一个时刻点上只有一个程序在处理机上运行。
   
### 并发管理：

竞态通常是对资源的共享访问结果而产生。为了避免会发生的这种情况，在驱动设备程序中我们需要记住以下的几个规则：

1.只要可能就应该避免资源的共享，但是对于硬件资源来说，它本质上就是共享的，而软件资源经常需要对其他线程可用。

2.对于硬件资源来说，必须使用显示的管理方式。

3.当内核代码创建了一个可能和其他内核部分贡献的对象时，该对象必须在还有其他组件引用自己时保持存在。

### Linux Kernel 信号量的实现

我们在”linux/semaphore.h“ 中可以看到具体的结构和基本函数，在当前4.4内核中已经作出了比2.6.x 更多的改变。

```
/* Please don't access any members of this structure directly */
struct semaphore {                /*信号量的结构*/
	raw_spinlock_t		lock;
	unsigned int		count;
	struct list_head	wait_list;  /*等待链表*/
};

#define __SEMAPHORE_INITIALIZER(name, n)				\
{									\
	.lock		= __RAW_SPIN_LOCK_UNLOCKED((name).lock),	\
	.count		= n,						\
	.wait_list	= LIST_HEAD_INIT((name).wait_list),		\
}

#define DEFINE_SEMAPHORE(name)	\
	struct semaphore name = __SEMAPHORE_INITIALIZER(name, 1)

static inline void sema_init(struct semaphore *sem, int val) /*初始化函数*/
{
	static struct lock_class_key __key;
	*sem = (struct semaphore) __SEMAPHORE_INITIALIZER(*sem, val);
	lockdep_init_map(&sem->lock.dep_map, "semaphore->lock", &__key, 0);
}

extern void down(struct semaphore *sem);    /*减少信号量的值，调用者等待*/
extern int __must_check down_interruptible(struct semaphore *sem); /*可中断的*/
extern int __must_check down_killable(struct semaphore *sem);      /*可被终结的*/
extern int __must_check down_trylock(struct semaphore *sem);       /*尝试加锁，永不休眠，获取资源失败直接返回*/
extern int __must_check down_timeout(struct semaphore *sem, long jiffies); /*定时*/
extern void up(struct semaphore *sem);     /*释放信号量*/

```

### 读取者 写入者 信号量

对于有些资源大多处于读取状态的时候，内核提供了一个读写者信号量，允许多个并发的读取者提高了性能。

```
struct rw_semaphore;    /*读写信号量的结构*/

/* All arch specific implementations share the same struct */
struct rw_semaphore {
	long count;
	struct list_head wait_list;
	raw_spinlock_t wait_lock;
	struct optimistic_spin_queue osq; /* spinner MCS lock */
	/*
	 * Write owner. Used as a speculative check to see
	 * if the owner is running on the cpu.
	 */
	struct task_struct *owner;

	struct lockdep_map	dep_map;

};

extern struct rw_semaphore *rwsem_down_read_failed(struct rw_semaphore *sem);
extern struct rw_semaphore *rwsem_down_write_failed(struct rw_semaphore *sem);
extern struct rw_semaphore *rwsem_wake(struct rw_semaphore *);
extern struct rw_semaphore *rwsem_downgrade_wake(struct rw_semaphore *sem);
```

#### 初始化函数的基本操作，包括初始化的宏定义

```
#define __RWSEM_INITIALIZER(name)				\
	{ .count = RWSEM_UNLOCKED_VALUE,			\
	  .wait_list = LIST_HEAD_INIT((name).wait_list),	\
	  .wait_lock = __RAW_SPIN_LOCK_UNLOCKED(name.wait_lock)	\
	  __RWSEM_OPT_INIT(name)				\
	  __RWSEM_DEP_MAP_INIT(name) }

#define DECLARE_RWSEM(name) \
	struct rw_semaphore name = __RWSEM_INITIALIZER(name)

extern void __init_rwsem(struct rw_semaphore *sem, const char *name,
			 struct lock_class_key *key);

#define init_rwsem(sem)						\
do {								\
	static struct lock_class_key __key;			\
								\
	__init_rwsem((sem), #sem, &__key);			\
} while (0)

/*
 * This is the same regardless of which rwsem implementation that is being used.
 * It is just a heuristic meant to be called by somebody alreadying holding the
 * rwsem to see if somebody from an incompatible type is wanting access to the
 * lock.
 */
static inline int rwsem_is_contended(struct rw_semaphore *sem)
{
	return !list_empty(&sem->wait_list);
}
```


#### 读写信号量的基本操作

```
static inline int rwsem_is_contended(struct rw_semaphore *sem)
{
	return !list_empty(&sem->wait_list);
}

/*
 * lock for reading
 */
extern void down_read(struct rw_semaphore *sem);

/*
 * trylock for reading -- returns 1 if successful, 0 if contention
 */
extern int down_read_trylock(struct rw_semaphore *sem);

/*
 * lock for writing
 */
extern void down_write(struct rw_semaphore *sem);

/*
 * trylock for writing -- returns 1 if successful, 0 if contention
 */
extern int down_write_trylock(struct rw_semaphore *sem);

/*
 * release a read lock
 */
extern void up_read(struct rw_semaphore *sem);

/*
 * release a write lock
 */
extern void up_write(struct rw_semaphore *sem);

```
一个读写信号量允许一个写入者或者无限多读取者拥有信号量，写入者拥有更高的优先级，当一个写入者在进入临界区工作的时候，直到其他的读操作完成之前是不会让读者进行操作的。

#### completion 

这是一个允许条件唤醒阻塞的操作，我们不必面面据道这里不进行深入讨论。有一个内核模块在本章。

### 自旋锁

一个核心概念，自旋锁大多用于SMP结构，建议最好在短时间可控制的情况下使用自旋锁。它是为实现保护共享资源而提出一种锁机制。其实，自旋锁与互斥锁比较类似，它们都是为了解决对某项资源的互斥使用。无论是互斥锁，还是自旋锁，在任何时刻，最多只能有一个保持者，也就说，在任何时刻最多只能有一个执行单元获得锁。但是两者在调度机制上略有不同。对于互斥锁，如果资源已经被占用，资源申请者只能进入睡眠状态。但是自旋锁不会引起调用者睡眠，如果自旋锁已经被别的执行单元保持，调用者就一直循环在那里看是否该自旋锁的保持者已经释放了锁，"自旋"一词就是因此而得名。

```
static __always_inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
}

#define spin_lock_init(_lock)				\
do {							\
	spinlock_check(_lock);				\
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

static __always_inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
}

static __always_inline void spin_lock_bh(spinlock_t *lock)
{
	raw_spin_lock_bh(&lock->rlock);
}

static __always_inline int spin_trylock(spinlock_t *lock)
{
	return raw_spin_trylock(&lock->rlock);
}

```

#### Linux kernel 原子变量 与 位操作

原子变量很简单参见"include/linux/atomic.h"

```
typedef struct {  
    volatile int counter;  
} atomic_t;  

```

```
static inline int atomic_read(const atomic_t *v)  
{  
    return v->counter;  
}  
  
static inline void atomic_set(atomic_t *v, int i)  
{  
    v->counter = i;  
}  
```

### RCU 机制

这里有一个关于RCU的论文合集，由内核RCU的编写者整理，读者可自行阅读http://www2.rdrop.com/users/paulmck/RCU/

