/*
*********************************************************************************************************
*                                          Sword Light
*
*                                       齐鲁软件设计大赛
*
*                               济南职业学院08软件班韶光队参赛作品
* 
* 文件名称	：SLOS.H
* 创建日期	：2009/5/14
* 作者		：张养静
* 说明		：系统中各种数据结构，全局变量的定义
*********************************************************************************************************
*/

#include    "Includes.h"


/*
*********************************************************************************************************
*                                       常量宏定义
*
*********************************************************************************************************
*/

/* 对就绪标志组以及就绪标志位图长度的定义  */
#if MIN_PRIO <= 63
#define  OS_EVENT_TBL_SIZE ((MIN_PRIO) / 8 + 1)		//事件等待任务位图的大小
#define  OS_RDY_TBL_SIZE   ((MIN_PRIO) / 8 + 1)		//任务就绪表位图的大小
#else
#define  OS_EVENT_TBL_SIZE ((MIN_PRIO) / 16 + 1)	//事件等待任务位图的大小
#define  OS_RDY_TBL_SIZE   ((MIN_PRIO) / 16 + 1)	//任务就绪表位图的大小
#endif

/* OS_TCB->State 的可选值  */
#define  OS_STATE_READY          0x00				//任务就绪
#define  OS_STATE_FREE           0x01				//空闲的TCB
#define  OS_STATE_MBOX           0x02				//等待邮箱
#define  OS_STATE_CREATED        0x04				//任务创建，但还未执行taskStart()让其就绪
#define  OS_STATE_SUSPEND        0x08				//任务挂起
#define  OS_STATE_MUTEX          0x10				//等待互斥型信号量	
#define  OS_STATE_DEALY          0x40				//任务延时


/* OS_EVENT->OSEventType 的可选值  */
#define  OS_EVENT_TYPE_UNUSED		0u
#define  OS_EVENT_TYPE_MBOX			1u
#define  OS_EVENT_TYPE_Q			2u
#define  OS_EVENT_TYPE_SEM			3u
#define  OS_EVENT_TYPE_MUTEX		4u
#define  OS_EVENT_TYPE_FLAG			5u

/* OS_EVENT->State 的可选值  */ 
#define  OS_MUTEX_STATE_FREE		0u				//没有被使用，没有被take
#define  OS_MUTEX_STATE_TAKE		1u				//被take，但还没有另一个任务在等待
#define  OS_MUTEX_STATE_WAIT		2u				//有任务在等待


/*         各种错误代码            */
#define SLOS_ERR_FAIL				-1
#define SLOS_ERR_NO_TCB        		-3

#define SLOS_ERR_NO_ERR        		0
#define SLOS_ERR_TIME_OUT			1
#define SLOS_ERR_MBOX_FUL			2

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif

/*
*********************************************************************************************************
*                                       任务控制块
*						              OS_TCB
*********************************************************************************************************
*/

#ifndef SwordLight
#define SwordLight

typedef struct os_tcb
{

	OS_STK			*OSTCBStkPtr;			//当前TCB的栈顶的指针，放在tcb最前面是因为为了任务调度时方便取得堆栈地址	
	
	struct os_tcb	*NextTcb;				//构建tcb空闲链表用到的指针
	struct os_tcb	*LastTcb;				//构建tcb空闲链表用到的指针

	INT8U			Prio;					//当前任务的优先级
	INT8U			State;					//当前任务的任务状态
	INT16U			TaskId;					//当期任务的任务id号
	INT8S			Name[TASK_NAME_LEN];	//当前任务的名字

	INT8U            OSTCBX;				//在就绪标志组中的位置
    INT8U            OSTCBY;				//在就绪位图中的位置
	
	#if MIN_PRIO <= 63
	INT8U			 OSTCBBitX; 			//在就绪标志组里的标志位
	INT8U			 OSTCBBitY; 			//在就绪位图里的标志位
	#else
	INT16U			 OSTCBBitX; 			//在就绪标志组里的标志位
	INT16U			 OSTCBBitY; 			//在就绪位图里的标志位
	#endif

	#if OS_EVENT_EN >0
	struct os_tcb	*ENextTcb;				//构建任务控制块就绪分级表用到的指针
	struct os_tcb	*ELastTcb;				//构建任务控制块就绪分级表用到的指针
	INT8U			Request;				//请求是否得到，还是超时
	struct os_event	*Eventptr;				//指向事件控制块的指针，标志此TCB关于mutex，mbox等的状态
	#endif
	
	#if STATIC_MALLOC_EN == 0 && OS_TASK_DEL_EN == 1
	void			*StkPtr;				//使用malloc得到的堆栈地址，供释放堆栈用
	#endif
	
	#if TASK_DELAY_EN > 0
	INT16U			Delay;					//当前任务的延时时间
	#endif

	#if TASK_SCHED_TIME_EN > 0				//如果允许同级时间片轮转
	INT8U			TimeLen;				//当前任务每次轮转要使用的时间片总数
	INT8U			CurTime;				//当前任务当前轮转还剩下多少时间片。此参数初始值和TimeLen相同	
	#endif
	
	#if TASK_STATA_EN > 0					//如果允许任务信息统计
	void			*statptr;
	#endif

}OS_TCB;

/*
*********************************************************************************************************
*                                       堆栈使用信息结构
*						               OS_STKINFO
*********************************************************************************************************
*/
typedef struct os_stkinfo {
	INT16U	StkUsed;
	INT16U	StkFree;
} OS_STKINFO;

/*
*********************************************************************************************************
*                                       事件控制块
*						             OS_EVENT
*********************************************************************************************************
*/

typedef struct os_event {
	INT16U	EventId;
    INT8U	OSEventType;                    //事件控制块的类型
    void    *OSEventPtr;					//指向邮箱的邮件或者消息队列的队列控制块
    INT16U	OSEventCnt;                     //对信号量的计数
#if MIN_PRIO <= 63
    INT8U	OSEventGrp;                     //就绪标志组
    INT8U	OSEventTbl[OS_EVENT_TBL_SIZE];  //就绪标志位图
#else
    INT16U	OSEventGrp;                     //就绪标志组
    INT16U	OSEventTbl[OS_EVENT_TBL_SIZE];  //就绪标志位图
#endif

	OS_TCB	*SLOS_tcb_by_prio[MIN_PRIO + 1];//优先级分级链表的各优先级的表头指针
	
#if TASK_STATA_EN > 0						//如果允许任务信息统计
	INT8U	State;
#endif

#if OS_EVENT_NAME_SIZE > 0
    INT8U    OSEventName[OS_EVENT_NAME_SIZE];//事件的名字
#endif
} OS_EVENT;




/*
*********************************************************************************************************
*                                       函数头定义
*
*********************************************************************************************************
*/

/******  SLOS_Core.c  *******/
INT8S		taskInit(void);						//初始化操作系统，建立多任务环境. 该函数调用后，其他系统调用则可用
INT16U 		taskCreate(INT8U *name, void *entryPtr, INT16U stack_size, INT8U priority);	//创建一个任务
INT8S 		taskStart(INT16U tid);				//启动一个任务, 让taskCreat创建的任务参加调度
INT8S		taskDelete(INT16U tid);				//将任务删除

#if TASK_SUSPEND_EN > 0
INT8S		taskSuspend(INT16U tid);			//挂起任务
INT8S		taskResume(INT16U tid);				//恢复挂起的任务
#endif

#if GET_SET_PRIO_EN	> 0
INT8U		taskPriorityGet(INT16U tid);		//获取任务的优先级
INT8S		taskPrioritySet(INT16U tid, INT8U newPriority);	//设置任务的优先级,用于动态调整
#endif

#if TASK_DELAY_EN > 0
INT8S 		taskSleep(INT16U ticks);			//将一个任务延时
#endif

void		SLOS_free_task(void *pdd);			//系统的空闲任务
void		SLOS_start(void);					//系统进入多任务
OS_TCB		*SLOS_tcb_init(void (*task)(void *pd), INT8U *name, INT8U prio);	//tcb结构初始化
void		SLOS_task_sched(void);				//进行任务调度
void		OSIntEnter(void);							
void 		OSTimeTick(void);					//时钟中断处理
void 		OSIntExit(void);

void 		slos_insert_ready(OS_TCB *ptcb);						//将TCB插入到就绪链表
void 		slos_out_ready(OS_TCB *ptcb);							//将一个任务脱离就绪链表
void		SLOS_get_heighready(void);								//得到当前就绪任务的最高的优先级
void		SLOS_event_waitlist_init(OS_EVENT *pevent);				//初始化时间控制块的等待任务位图
void 		SLOS_event_task_ready(OS_EVENT *pevent, void *pmsg);	//使一个任务进入就绪状态
void 		SLOS_event_task_wait(OS_EVENT *pevent);					//使一个任务进入等待某事件发生状态
#if TASK_STATA_EN > 0
void		SLOS_state_task(void *pdd);								//系统的统计任务
void		SLOS_state_init(void);									//初始化系统的统计任务
void		staticCheck(INT16U tid,OS_STKINFO *pstkinfo);			//检测任务的堆栈使用情况
#endif

/******  SLOS_MUTEX.C  *******/
#if MUTEX_EN > 0
INT16U 		semCreate(void);										//从信号量控制块空闲列表中创建一个信号量控制块
INT8S 		semTake(INT16U sid);									//请求一个互斥型信号量
INT8S 		semGive(INT16U sid);									//释放一个互斥型信号量
#endif

/******  SLOS_MBOX.C  *******/
#if MBOX_EN > 0
INT16U		mboxCreate(void);								//消息邮箱的创建
INT8S		mboxPost(INT16U mid,void *msg);					//发送一则消息
void		*mboxPend(INT16U mid,INT16U timeout,INT8U *err);//等待一则消息
#endif

/********     SLOS_CPU.H   **********/
OS_STK		*OSTaskStkInit(void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt);
void		OSStartHighRdy(void);
void		OSIntCtxSw(void);
void		OSCtxSw(void);
void		OSTickISR(void);										//时钟中断处理

#if CPU_HOOKS_EN > 0
void		OSInitHookBegin(void);									//各种用户hook，可以在SL.H中定义是否使用，及是否由用户自己定义
void		OSInitHookEnd(void);
void		OSTaskCreateHook(OS_TCB *ptcb);
void		OSTaskDelHook(OS_TCB *ptcb);
void		OSTaskIdleHook(void);
void		OSTaskStatHook(void);
void		OSTaskSwHook(void);
void		OSTCBInitHook(OS_TCB *ptcb);
void		OSTimeTickHook(void);
#endif


/*
*********************************************************************************************************
*                                       全局变量
*						      
*********************************************************************************************************
*/
#ifdef quan_ju				//定义全局和局部的区分用到的宏
#define EXT
#else
#define EXT extern
#endif

#if MIN_PRIO<= 63										//如果总任务数小于64，使用8*8的就绪位图。若大于64，则使用16*16的位图。
EXT  INT8U				OSRdyGrp;						//任务就绪组
EXT  INT8U				OSRdyTbl[OS_RDY_TBL_SIZE];		//任务就绪表
#else
EXT  INT16U				OSRdyGrp;						//任务就绪组
EXT  INT16U				OSRdyTbl[OS_RDY_TBL_SIZE];		//任务就绪表
#endif

extern INT8U   const	OSUnMapTbl[256];         		//用于反查一个字节数字最高是1的位的位置的快表数组  这个数组是在SLOS_Core.c中定义的

EXT 	OS_TCB			SLOS_tcb[TASK_COUNT];			//定义OS_TCB任务控制块的空间
EXT 	OS_TCB			*SLOS_tcb_free_list;			//定义全局的TCB链表的表头指针，用来存储空闲tcb

#if OS_EVENT_EN >0
EXT 	OS_EVENT		SLOS_event[OS_EVENT_COUNT];		//定义OS_EVENT任务控制块的空间
EXT  	OS_EVENT		*SLOS_event_free_list;			//定义全局的event链表的表头指针，用来存储空闲event
#endif


EXT OS_TCB				*SLOS_tcb_delay_list;			//定义全局的TCB链表的表头指针，用来存储需要延时的tcb

EXT OS_TCB				*SLOS_tcb_suspend_list;			//定义全局的TCB链表的表头指针，用来存储需要挂起的tcb

EXT OS_TCB				*OSTCBHighRdy;					//下一个即将运行的任务，任务调度时用

EXT OS_TCB				*OSTCBCur;						//当前正在运行的的TCB的指针

EXT OS_TCB				*SLOS_tcb_by_prio[MIN_PRIO + 1];//根据优先级区分的分优先级链表的各链表的头指针，用于通过就绪位图找到最好优先级的就绪任务

EXT INT16U				SLOS_Prio_Cur;					//当前运行的任务的优先级

EXT INT8U				SLOS_Prio_highready;			//当前就绪任务的最高的优先级

EXT INT32U 				free_task_count;				//空闲任务的计数值，空闲任务的任务就是不断的加这个数


#if TASK_STATA_EN > 0
EXT INT16U				time_tick_count;				//中断的计数值，记录一共中断了多少次

EXT INT32U				task_switch_count;				//任务交换的计数值

EXT INT16U				task_count;						//任务的数量计数

EXT INT32U				cpu_count_max;					//cpu每一段时间间隔空闲任务的计数

EXT INT8S				cpu_Usage;						//cpu的使用率

EXT BOOLEAN				OS_Stat_Rdy;					//是否准备好进行统计
#endif


EXT  BOOLEAN			OSRunning;						//标志系统已经开始运行

EXT  INT8U				OSIntNesting;					//中断计数

#if STATIC_MALLOC_EN > 0
EXT	 OS_STK				SLOS_stk[TASK_COUNT][TASK_STATIC_SIZE];		//静态分配任务的堆栈空间
#endif

#if FIT_UCOS_HARDWARE > 0								//这是为了适应ucos的硬件抽象层的代码，内核中并未使用的变量
EXT  INT8U             OSPrioCur;						//当前任务优先级
EXT  INT8U             OSPrioHighRdy;					//就绪任务最高优先级
#endif

#endif
