/*
*********************************************************************************************************
*                                          Sword Light
*
*                                       齐鲁软件设计大赛
*
*                               济南职业学院08软件班韶光队参赛作品
* 
* 文件名称	：SL_CFG.H
* 创建日期	：2009/5/14
* 作者		：张养静
* 说明		：操作系统的配置文件
*********************************************************************************************************
*/

#ifndef SF_CFG
#define SF_CFG



#define	FIT_UCOS_HARDWARE	1				//定义是否适合ucos的硬件抽象层 主要是在SLOS.H中定义了常量OSPrioCur，OSPrioHighRdy以适应ucos的硬件抽象层代码


#define CPU_HOOKS_EN		1				//定义是否允许hook函数

	#define USE_OS_HOOKS	1				//定义是否使用系统自带的HOOK函数


#define GET_SET_PRIO_EN		1				//定义是否允许获取,设置优先级


#define TASK_SCHED_TIME_EN	1				//定义是否允许同级时间片轮转
	
	#define TASK_TIME_LEN	1				//定义同级时间片轮转的时间片数

#define TASK_DELAY_EN		1				//定义是否允许任务的延时

#define OS_TASK_DEL_EN		1				//定义是否允许删除任务

#define OS_EVENT_EN			1				//定义是否允许事件

	#define OS_EVENT_COUNT		1			//定义事件控制块的数量

	#define EVENT_DELAY			1			//定义mutex等待信号量时的等待时间，这是防止死锁的方法。由于题目api没有相应参数，故在此设置。0是永远等待

	#define OS_EVENT_NAME_SIZE	1			//定义事件控制块的名字，0表示不使用事件名字

	#define MUTEX_EN			1			//定义是否允许互斥型信号量 		

	#define MBOX_EN				1			//定义是否允许消息邮箱

#define TASK_SUSPEND_EN		1				//定义是否允许任务的挂起与恢复

#define TASK_STATA_EN		1				//定义是否允许任务状态信息统计

#define TASK_NAME_LEN		1				//任务名字的长度

#define MIN_PRIO			1				//定义最低优先级，即空闲任务的优先级,也就是优先级的总数

#define TASK_COUNT			1				//定义系统的任务总数，包括空闲任务,统计任务(如果允许使用统计功能的话)



#define	STATIC_MALLOC_EN	1				//定义是否允许静态内存分配

	#define TASK_STATIC_SIZE	1			//定义静态分配时每个任务的堆栈的大小，单位OS_STK

	#define FREE_TASK_STK_SIZE	1			//定义动态分配时空闲任务的堆栈大小，单位byte

	#define STATE_TASK_STK_SIZE	1			//定义动态分配时统计任务的堆栈大小，单位byte


#define OS_TICKS_PER_SEC	1				//定义系统没每秒钟的中断次数


/************   针对用户的定义进行的一些调整(用户无需设置)  ******************/
#if CPU_HOOKS_EN == 0						//如果已经禁止使用hooks，那么USE_OS_HOOKS没有意义，必须为0
	#if defined USE_OS_HOOKS
    	#undef USE_OS_HOOKS
    #endif
	#define USE_OS_HOOKS		0
#endif	

#if USE_OS_HOOKS == 0						//这是针对ucos的硬件抽象层的一个转换
	#if defined OS_CPU_HOOKS_EN
		#undef  OS_CPU_HOOKS_EN	
	#endif
	#define OS_CPU_HOOKS_EN		0
#endif

#if USE_OS_HOOKS == 1	
    #define OS_VERSION 		252				//此系统包含的hooks是ucos2.52版本中所包含的
    #if defined OS_CPU_HOOKS_EN
		#undef  OS_CPU_HOOKS_EN
	#endif
	#define OS_CPU_HOOKS_EN	1
#endif

#if OS_EVENT_EN == 0						//如果禁止了事件控制块，那么也必须禁止会用到事件控制块的内容
	#undef	OS_EVENT_COUNT
	#undef	OS_EVENT_NAME_SIZE
	#undef	MUTEX_EN
	#undef	MBOX_EN

	#define OS_EVENT_COUNT		0
	#define OS_EVENT_NAME_SIZE	0
	#define MUTEX_EN			0
	#define MBOX_EN				0
#endif

#ifndef OS_CRITICAL_METHOD					//某些硬件抽象层没有定义此宏定义
	#define OS_CRITICAL_METHOD	1
#endif




#endif
