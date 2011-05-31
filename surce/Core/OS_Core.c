/*
*********************************************************************************************************
*                                          Sword Light
*
*                                       齐鲁软件设计大赛
*
*                               济南职业学院08软件班韶光队参赛作品
* 
* 文件名称	：SLOS_Core.c
* 创建日期	：2009/5/14
* 作者		：张养静
* 说明		：系统中关键的算法代码实现
*********************************************************************************************************
*/
#ifndef quan_ju
    #define quan_ju
#endif

#include "Includes.h"	//包含系统的头文件

/*
****************************************************************************************
*
*                              系统常量的定义
* 
****************************************************************************************
*/

INT8U   const  OSUnMapTbl[256] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/*
****************************************************************************************
*                                     系统的空闲任务
*
* 函数名称	：SLOS_free_task(void *pdd)
* 创建日期	：2009/5/19
* 修改日期	：2009/8/8
* 说明		：操作系统的空闲任务。最低优先级运行，且系统运行期间始终存在
* 参数		：	pdd		:传给任务的参数
* 返回		：无
****************************************************************************************
*/
void SLOS_free_task(void *pdd)
{
	#if OS_CRITICAL_METHOD == 3			//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 

	pdd = pdd;
	
	while(1){
		OS_ENTER_CRITICAL();
		free_task_count++;				//空闲任务的任务就是把这个全局的计数不断的进行加一
		OS_EXIT_CRITICAL();

		#if CPU_HOOKS_EN > 0
		OSTaskIdleHook();
		#endif
	}
}

/*
****************************************************************************************
*                                     系统的统计任务
*
* 函数名称	：SLOS_state_task()
* 创建日期	：2009/7/21
* 修改日期	：2009/7/21
* 说明		：操作系统的统计任务。次最低优先级运行，可通过配置选择是否运行
* 参数		：无
* 返回		：无
****************************************************************************************
*/
#if TASK_STATA_EN > 0
void SLOS_state_task(void *pdd)
{	
	pdd = pdd;

	while (OS_Stat_Rdy == FALSE) {
        taskSleep(2 * OS_TICKS_PER_SEC);
    }
	
	
	while(1){		
		cpu_Usage = (INT8S)(100 - free_task_count*100/cpu_count_max);
		if(cpu_Usage < 0)
			cpu_Usage = 1;
		if(cpu_Usage > 100)
			cpu_Usage = 100;
		free_task_count = 0;

		#if CPU_HOOKS_EN > 0
		OSTaskStatHook();
		#endif
		
		taskSleep(OS_TICKS_PER_SEC);
	}
}
#endif

/*
****************************************************************************************
*                                     系统的统计任务初始化
*
* 函数名称	：SLOS_state_init()
* 创建日期	：2009/7/21
* 修改日期	：2009/7/21
* 说明		：操作系统的统计任务。次最低优先级运行，可通过配置选择是否运行
* 参数		：无
* 返回		：无
****************************************************************************************
*/
#if TASK_STATA_EN > 0
void SLOS_state_init()
{
	#if OS_CRITICAL_METHOD == 3		//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	OS_ENTER_CRITICAL();
	free_task_count = 0;			//为获取cpu使用率做准备
	OS_EXIT_CRITICAL();
	taskSleep(OS_TICKS_PER_SEC);
	OS_ENTER_CRITICAL();
	cpu_count_max= free_task_count;
	OS_EXIT_CRITICAL();
	OS_Stat_Rdy = TRUE;	
}
#endif

/*
****************************************************************************************
*                                     系统的初始化
*
* 函数名称	：INT8S taskInit(void)
* 创建日期	：2009/5/15
* 修改日期	：2009/7/30
* 说明		：初始化操作系统，建立多任务环境. 该函数调用后，其他系统调用则可用
* 参数		：无			
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
INT8S taskInit()	//系统初始化
{  	
	INT16U i;				//初始空闲链表时使用
	INT8U j;				//初始就绪位图时使用
	OS_TCB		*ptcb1;		//定义循环构建链表用的两个临时TCB指针
	OS_TCB		*ptcb2;	
	#if OS_EVENT_COUNT > 1
	OS_EVENT	*pevent1;	//定义循环构建链表用的两个临时EVENT指针
	OS_EVENT	*pevent2;
	#endif
	#if MIN_PRIO<= 63		//初始化就绪位图表
    INT8U   	*prdytbl;	//定义一个指向就绪位图表的指针
	#else
    INT16U  	*prdytbl;
	#endif
	INT16U		tid;		//错误代码，获取创建任务的返回值

	#if CPU_HOOKS_EN > 0
	OSInitHookBegin();
	#endif
	
	SLOS_tcb_delay_list		= (OS_TCB*)0;
	SLOS_tcb_suspend_list	= (OS_TCB*)0;
	OSTCBCur				= (OS_TCB*)0;
	OSTCBHighRdy			= (OS_TCB*)0;	
	OSRunning				= FALSE;
	SLOS_Prio_Cur     		= 0; 			//当前的任务优先级初始化为0
	OSIntNesting 			= 0;	

	for(i = 0;i < MIN_PRIO + 1;i++){		//初始化分级链表的表头为0
		SLOS_tcb_by_prio[i] = (OS_TCB*)0;
	}

	SLOS_tcb_free_list	= SLOS_tcb;			//初始化空闲TCB链表的头指针
	ptcb1				= &SLOS_tcb[0];		//之所以这样做是因为一个完整的系统至少会有两个任务
	ptcb2				= &SLOS_tcb[1];
	ptcb1->TaskId		= 0;
	ptcb1->State		= OS_STATE_FREE;
	for(i = 0;i < TASK_COUNT - 1;i++){		//构建空闲TCB链表
		ptcb2->TaskId	= i + 1;
		ptcb2->State	= OS_STATE_FREE;
		ptcb1->NextTcb	= ptcb2;
		ptcb1++;
		ptcb2++;		
	}
	
	#if OS_EVENT_EN >0						//初始化事件控制块链表,如果需要的话
	SLOS_event_free_list = &SLOS_event[0];	//初始化空闲链表的头指针	
	#if OS_EVENT_COUNT > 1					//只有在有多余1个事件控制块时才有必要组建链表
	pevent1	= &SLOS_event[0];
	pevent2	= &SLOS_event[1];
	pevent1->EventId = 0;
	for(i = 0;i < OS_EVENT_COUNT - 1;i++){	//构建空闲事件控制块链表
		pevent2->EventId	= i + 1;
		pevent1->OSEventPtr	= pevent2;
		pevent1++;
		pevent2++;
	}
	#else									//如果只有一个事件控制块,那么只需简单的处理下即可
	SLOS_event_free_list->EventId = 0;
	SLOS_event_free_list->OSEventPtr = (OS_EVENT *)0;
	#endif
	#endif
	
	OSRdyGrp      = 0;						//将就绪标志组以及就绪位图全部初始化为0
    prdytbl       = &OSRdyTbl[0];
    for (j = 0; j < OS_RDY_TBL_SIZE; j++) {
        *prdytbl++ = 0;
    }
	
	tid = taskCreate((void *)"free task", (void *)SLOS_free_task, FREE_TASK_STK_SIZE, MIN_PRIO);	//创建空闲任务
	if(tid < 0){
		return SLOS_ERR_FAIL;
	}
	taskStart((unsigned char)tid);
	
	#if TASK_STATA_EN > 0	
	tid = taskCreate((void *)"state task", (void *)SLOS_state_task, STATE_TASK_STK_SIZE, MIN_PRIO - 1);
	if(tid < 0){
		return SLOS_ERR_FAIL;
	}
	taskStart((unsigned char)tid);
	#endif

	#if CPU_HOOKS_EN > 0
	OSInitHookEnd();
	#endif

	return SLOS_ERR_NO_ERR;
}
/*
****************************************************************************************
*                                     创建一个任务
*
* 函数名称	：INT16U taskCreate(char *name, void *entryPtr, INT16U stack_size, INT16U priority)
* 创建日期	：2009/5/15
* 修改日期	：2009/7/31
* 说明		：创建一个任务
* 参数		：	name			: 任务名称
*				entryPtr		: 任务函数入口
*				stack_size		: 任务栈大小
*				priority		: 任务优先级
*			
* 返回		：任务编号tid, >0: 成功, 其他: 失败
****************************************************************************************
*/
INT16U taskCreate(INT8U *name, void *entryPtr, INT16U stack_size, INT8U priority)
{	
	OS_TCB		*ptcb;
	OS_STK		*pstk;
	INT16U		index;											//循环计数
	#if OS_CRITICAL_METHOD == 3									//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
		
	stack_size = stack_size;

	OS_ENTER_CRITICAL();
	ptcb = SLOS_tcb_init((void (*)(void *))entryPtr, (INT8U *)name, priority);	//初始化任务TCB结构
	if(ptcb == (OS_TCB *)0){
		OS_EXIT_CRITICAL();
		return SLOS_ERR_NO_TCB;
	}

	#if TASK_STATA_EN > 0										//如果任务需要进行堆栈检测，应将任务的堆栈全部清零
	for(index = 0;index < TASK_STATIC_SIZE;index++){
		SLOS_stk[ptcb->TaskId][index] = (OS_STK)0;
	}
	#endif


	#if STATIC_MALLOC_EN > 0									//如果系统采用静态内存分配

	#if OS_STK_GROWTH == 1										//如果堆栈是递减的，则调整指针到堆栈顶部
	pstk = &SLOS_stk[ptcb->TaskId][TASK_STATIC_SIZE - 1];
	#else
	pstk = &SLOS_stk[ptcb->TaskId][0];							//利用tcb的tid得到堆栈，堆栈数组和tcb数组是相对应的	
	#endif

	#else														//如果系统采用动态内存分配

	#if OS_STK_GROWTH == 1										//如果堆栈是递减的，则调整指针到堆栈顶部
	pstk = (OS_STK *)((INT16U)malloc(stack_size) + stack_size);
	#if OS_TASK_DEL_EN > 0										//允许任务删除，则记录得到的堆栈地址，供释放内存时用
	ptcb->StkPtr = (void *)((INT16U)pstk - stack_size);					//使用malloc得到的堆栈地址，供释放堆栈用
	#endif
	#else
	pstk = (OS_STK *)malloc(stack_size);						//利用tcb的tid得到堆栈，堆栈数组和tcb数组是相对应的	
	#if OS_TASK_DEL_EN > 0										//允许任务删除，则记录得到的堆栈地址，供释放内存时用
	ptcb->StkPtr = (void *)pstk;								//使用malloc得到的堆栈地址，供释放堆栈用
	#endif
	#endif
	

	#endif
	

	
	pstk = OSTaskStkInit((void (*)(void *))entryPtr, (void *)0, (OS_STK *)pstk, (INT16U)priority);	//初始化任务堆栈

	ptcb->OSTCBStkPtr = pstk;									//设置TCB中的堆栈指针	

	#if TASK_STATA_EN > 0
	task_count++;
	#endif
	
	OS_EXIT_CRITICAL();

	#if CPU_HOOKS_EN > 0
	OSTaskCreateHook(ptcb);
	#endif

	return ptcb->TaskId;										//这里干么要转换?题目问题。平台相关性
}

/*
****************************************************************************************
*                                    启动一个任务
*
* 函数名称	：INT8S taskStart(unsigned char tid)
* 创建日期	：2009/8/1
* 修改日期	：2009/8/1
* 说明		：启动一个任务, 让taskCreat创建的任务参加调度
* 参数		：	tid		:将要启动的任务的任务ID编号		
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
INT8S taskStart(INT16U tid){
	#if OS_CRITICAL_METHOD == 3					//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	OS_TCB	*ptcb;

	OS_ENTER_CRITICAL();
	ptcb	= &SLOS_tcb[tid];
	if(ptcb->State != OS_STATE_CREATED){
		OS_EXIT_CRITICAL();
		return SLOS_ERR_FAIL;
	}
	slos_insert_ready(ptcb);					//将任务插入到就绪分级链表		
	OS_EXIT_CRITICAL();

	if(OSRunning){								//只有系统真正跑起来时才进行调度		
		SLOS_task_sched();						//进行任务调度
		return SLOS_ERR_NO_ERR;	
	}

	#if TASK_STATA_EN == 0						//判断是否已经满足系统进入多任务状态的条件
	if(ptcb->Prio < MIN_PRIO){
		SLOS_start();	
	}
	#endif
	#if TASK_STATA_EN > 0
	if(ptcb->Prio < MIN_PRIO - 1){
		SLOS_start();	
	}
	#endif
	
	return SLOS_ERR_FAIL;						//因为系统还没有进入多任务状态，所以启动任务失败，但已经插入分级就绪链表中
}


/*
****************************************************************************************
*                                     删除一个任务
*
* 函数名称	：INT8S taskDelete(unsigned char tid)
* 创建日期	：2009/7/31
* 修改日期	：2009/7/31
* 说明		：将任务删除 该任务退出调度且回收任务分配的资源
* 参数		：	tid			: 任务的编号ID
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if OS_TASK_DEL_EN > 0
INT8S	taskDelete(INT16U tid)
{
	#if OS_CRITICAL_METHOD == 3			//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR	cpu_sr;
	#endif
	OS_TCB		*tcbdelete;
	#if OS_EVENT_EN > 0
	OS_EVENT	*pevent;
	#endif

	OS_ENTER_CRITICAL();
	tcbdelete = &SLOS_tcb[tid];	
	

	if(tcbdelete->Prio == MIN_PRIO){	//如果要删除的是空闲任务，则出错
		return SLOS_ERR_FAIL;
	}
	

	#if CPU_HOOKS_EN > 0
	OSTaskDelHook(tcbdelete);
	#endif

	#if OS_EVENT_EN > 0
	if(tcbdelete->State == OS_STATE_MUTEX || tcbdelete->State == OS_STATE_MBOX){	//如果此TCB在事件控制块的等待链表中
		pevent = tcbdelete->Eventptr;
		pevent->OSEventTbl[tcbdelete->OSTCBY] &= ~tcbdelete->OSTCBBitX;				//清除事件控制块的等待位图
	    if (pevent->OSEventTbl[tcbdelete->OSTCBY] == 0) {
	       	pevent->OSEventGrp &= ~tcbdelete->OSTCBBitY;
	    }	
		//从延时链表中删除此TCB
		if(tcbdelete->LastTcb != (OS_TCB *)0){						//判断是不是链表的第一个元素
			tcbdelete->LastTcb->NextTcb = tcbdelete->NextTcb; 		//如果不是第一个
		}else{
			SLOS_tcb_delay_list = tcbdelete->NextTcb;				//如果是第一个，要改链表头指针
		}
		if(tcbdelete->NextTcb != (OS_TCB *)0){						//判断是不是链表的最后一个元素
			tcbdelete->NextTcb->LastTcb = tcbdelete->LastTcb; 		//如果是，就不必设置下一个元素的LastTcb指针
		}	
		//脱离事件控制块的等待链表	
		if(tcbdelete->ELastTcb != (OS_TCB *)0){
			tcbdelete->ELastTcb->ENextTcb = tcbdelete->ENextTcb;
		}else{
			pevent->SLOS_tcb_by_prio[tcbdelete->Prio] = tcbdelete->ENextTcb;
		}
		if(tcbdelete->ENextTcb != (OS_TCB *)0){
			tcbdelete->ENextTcb->ELastTcb = tcbdelete->ELastTcb;
		}
	}
	#endif	
	

	#if TASK_DELAY_EN > 0
	if(tcbdelete->State == OS_STATE_DEALY){						//如果此TCB正在延时，则脱离延时链表
		if(tcbdelete->LastTcb != (OS_TCB *)0){					//判断是不是链表的第一个元素
			tcbdelete->LastTcb->NextTcb = tcbdelete->NextTcb; 	//如果不是第一个
		}else{
			SLOS_tcb_delay_list = tcbdelete->NextTcb;			//如果是第一个，要改链表头指针
		}
		if(tcbdelete->NextTcb != (OS_TCB *)0){					//判断是不是链表的最后一个元素
			tcbdelete->NextTcb->LastTcb = tcbdelete->LastTcb; 	//如果是，就不必设置下一个元素的LastTcb指针
		}
	}
	#endif

	if(tcbdelete->State == OS_STATE_READY){						//如果此任务是在就绪状态，则脱离就绪分级链表
		slos_out_ready(tcbdelete);								//脱离就绪分级链表
	}	

	#if TASK_STATA_EN > 0
	task_count--;												//减少系统的任务计数
	#endif

	#if STATIC_MALLOC_EN == 0									//如果内存是动态分配的，则此处需要释放内存
	free(tcbdelete->StkPtr);											//使用malloc得到的堆栈地址，供释放堆栈用
	#endif
	
	tcbdelete->State   = OS_STATE_FREE;
	tcbdelete->NextTcb = SLOS_tcb_free_list;					//插入到空闲tcb链表
	SLOS_tcb_free_list = tcbdelete;
	
	OS_EXIT_CRITICAL();
	
	SLOS_task_sched();											//进行任务调度

	return SLOS_ERR_NO_ERR;
}
#endif

/*
****************************************************************************************
*                                     挂起一个任务
*
* 函数名称	：INT8S taskSuspend(unsigned char tid)
* 创建日期	：2009/5/17
* 修改日期	：2009/7/31
* 说明		：将一个任务挂起。从就绪链表移动到挂起链表
* 参数		：	tid		:将要挂起的任务的tcb指针		
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if TASK_SUSPEND_EN > 0
INT8S taskSuspend(INT16U tid)
{	
	#if OS_CRITICAL_METHOD == 3									//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif
	#if OS_EVENT_EN > 0
	OS_EVENT	*pevent;
	#endif
	OS_TCB	*tcbsuspend;
	
	OS_ENTER_CRITICAL();
	tcbsuspend	= &SLOS_tcb[tid];
	OS_EXIT_CRITICAL();

	if(tcbsuspend->State == OS_STATE_SUSPEND){
		return SLOS_ERR_FAIL;									//如果任务已经是挂起状态了，则返回错误
	}

	if(tcbsuspend->Prio == MIN_PRIO){
		return SLOS_ERR_FAIL;									//如果任务是空闲任务，则返回错误
	}

	if(tcbsuspend->State == OS_STATE_READY){					//挂起就绪状态的任务，出分级就绪链表	
		OS_ENTER_CRITICAL();
		slos_out_ready(tcbsuspend);								//将任务脱离分级就绪链表
		OS_EXIT_CRITICAL();		
	}

	OS_ENTER_CRITICAL();

	#if TASK_DELAY_EN > 0
	if(tcbsuspend->State == OS_STATE_DEALY){ 					//如果此TCB正在延时，则脱离延时链表
		if(tcbsuspend->LastTcb != (OS_TCB *)0){					//判断是不是链表的第一个元素
			tcbsuspend->LastTcb->NextTcb = tcbsuspend->NextTcb;	//如果不是第一个
		}else{
			SLOS_tcb_delay_list = tcbsuspend->NextTcb;			//如果是第一个，要改链表头指针
		}
		if(tcbsuspend->NextTcb != (OS_TCB *)0){					//判断是不是链表的最后一个元素
			tcbsuspend->NextTcb->LastTcb = tcbsuspend->LastTcb;	//如果是，就不必设置下一个元素的LastTcb指针
		}
	}
	#endif


	#if OS_EVENT_EN > 0
	if(tcbsuspend->State == OS_STATE_MUTEX || tcbsuspend->State == OS_STATE_MBOX){	//如果此TCB在事件控制块的等待链表中
		pevent = tcbsuspend->Eventptr;
		pevent->OSEventTbl[tcbsuspend->OSTCBY] &= ~tcbsuspend->OSTCBBitX; 			//清除事件控制块的等待位图
		if (pevent->OSEventTbl[tcbsuspend->OSTCBY] == 0) {
			pevent->OSEventGrp &= ~tcbsuspend->OSTCBBitY;
		}	
		//从延时链表中删除此TCB
		if(tcbsuspend->LastTcb != (OS_TCB *)0){						//判断是不是链表的第一个元素
			tcbsuspend->LastTcb->NextTcb = tcbsuspend->NextTcb;		//如果不是第一个
		}else{
			SLOS_tcb_delay_list = tcbsuspend->NextTcb;				//如果是第一个，要改链表头指针
		}
		if(tcbsuspend->NextTcb != (OS_TCB *)0){						//判断是不是链表的最后一个元素
			tcbsuspend->NextTcb->LastTcb = tcbsuspend->LastTcb;		//如果是，就不必设置下一个元素的LastTcb指针
		}	
		//脱离事件控制块的等待链表	
		if(tcbsuspend->ELastTcb != (OS_TCB *)0){
			tcbsuspend->ELastTcb->ENextTcb = tcbsuspend->ENextTcb;
		}else{
			pevent->SLOS_tcb_by_prio[tcbsuspend->Prio] = tcbsuspend->ENextTcb;
		}
		if(tcbsuspend->ENextTcb != (OS_TCB *)0){
			tcbsuspend->ENextTcb->ELastTcb = tcbsuspend->ELastTcb;
		}
	}
	#endif

	
	tcbsuspend->State 		= OS_STATE_SUSPEND;			//修改任务状态为挂起状态
	
	tcbsuspend->LastTcb		= (OS_TCB *)0;				//为出挂起链表做准备
	tcbsuspend->NextTcb		= SLOS_tcb_suspend_list;	//将TCB加入到挂起TCB链表
	if(tcbsuspend->NextTcb != (OS_TCB *)0){				//如果原来的挂起链表已经有挂起TCB了
		tcbsuspend->NextTcb->LastTcb = tcbsuspend;
	}
	SLOS_tcb_suspend_list	= tcbsuspend;
	
	OS_EXIT_CRITICAL();
	

	
	SLOS_task_sched();									//进行任务调度

	return SLOS_ERR_NO_ERR;
}
#endif

/*
****************************************************************************************
*                                    恢复挂起的一个任务
*
* 函数名称	：SLOS_task_resume(OS_TCB *tcbsuspend)
* 创建日期	：2009/5/17
* 修改日期	：2009/7/31
* 说明		：将一个挂起的任务恢复。从挂起链表移动到就绪链表。
* 参数		：	tid		:将要恢复的任务的任务号tid	
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if TASK_SUSPEND_EN > 0
INT8S taskResume(INT16U tid)
{
  	#if OS_CRITICAL_METHOD == 3								//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	OS_TCB	*tcbresume;

	OS_ENTER_CRITICAL();
	tcbresume	= &SLOS_tcb[tid];
	OS_EXIT_CRITICAL();
	
	if(tcbresume->State != OS_STATE_SUSPEND){				//只有是已经被挂起的任务才可以进行恢复
		return SLOS_ERR_FAIL;
	}	
	
	OS_ENTER_CRITICAL();	

	if(tcbresume->LastTcb != (OS_TCB*)0){					//把要恢复的tcb从挂起链表中脱离
		tcbresume->LastTcb->NextTcb = tcbresume->NextTcb;
	}else{
		SLOS_tcb_suspend_list = tcbresume->NextTcb;			//上一个为0，说明这是链表的第一个，所以要设置链表头指向下一个
	}
	if(tcbresume->NextTcb != (OS_TCB*)0){
		tcbresume->NextTcb->LastTcb = tcbresume->LastTcb;
	}
	
	slos_insert_ready(tcbresume);							//插入到就绪链表中

	tcbresume->State		= OS_STATE_READY;				//将TCB的状态设置为就绪状态
	
	OS_EXIT_CRITICAL();
	
	SLOS_task_sched();

	

	return SLOS_ERR_NO_ERR;

}
#endif
/*
****************************************************************************************
*                                   延时一个任务
*
* 函数名称	：INT8S taskSleep(INT16U ticks)
* 创建日期	：2009/5/29
* 修改日期	：2009/7/31
* 说明		：任务睡眠(非忙等), 由任务自己调用, 主动阻塞一段时间. 时间单位为tick, 即10毫秒
* 参数		：	ticks	:要延时的时间的长度,单位为tick			
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if TASK_DELAY_EN > 0
INT8S taskSleep(INT16U ticks)
{
	#if OS_CRITICAL_METHOD == 3					//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	INT16U	time;

	if(ticks == 0){								//如果延时时间是0，则是永远不会唤醒，这是不允许的。而延时0也就是不延时，可以直接返回。 为0永远延时是为互斥信号量设计的
		return SLOS_ERR_FAIL;
	}
	
	OS_ENTER_CRITICAL();
	time = (INT16U)ticks;
	
	slos_out_ready(OSTCBCur);					//将当前任务脱离就绪链表

	OSTCBCur->State = 0;						//设置任务的状态
	OSTCBCur->State = OS_STATE_DEALY;
	
	
	OSTCBCur->LastTcb = (OS_TCB *)0;			//为出延时链表做准备
	OSTCBCur->NextTcb = SLOS_tcb_delay_list;	//插入到延时链表中		
	
	if(OSTCBCur->NextTcb != (OS_TCB *)0){		//如果原来的延时链表已经有延时tcb了
		OSTCBCur->NextTcb->LastTcb = OSTCBCur;	//做成双向链表
	}
	SLOS_tcb_delay_list = OSTCBCur;

	OSTCBCur->Delay = time;						//设置延时时间

	OS_EXIT_CRITICAL();

	SLOS_task_sched();							//进行任务调度

	

	return SLOS_ERR_NO_ERR;
	
}
#endif

/*
****************************************************************************************
*                                    任务调度
*
* 函数名称	：SLOS_task_sched()
* 创建日期	：2009/5/17
* 修改日期	：2009/5/17
* 说明		：判断当前任务是否仍是最高优先级。不是的话进行调度运行最高优先级的任务。
* 参数		：无
* 返回		：无
****************************************************************************************
*/
void SLOS_task_sched()
{
	#if OS_CRITICAL_METHOD == 3								//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	OS_ENTER_CRITICAL();
	
	SLOS_get_heighready();									//找到当前最高优先级的任务

	if(OSTCBCur != SLOS_tcb_by_prio[SLOS_Prio_highready]){	//只有当最高优先级任务不是当前任务的情况下才进行任务切换

		#if CPU_HOOKS_EN > 0
		OSTaskSwHook();
		#endif
	
		OSTCBHighRdy = SLOS_tcb_by_prio[SLOS_Prio_highready];

		#if TASK_STATA_EN > 0
		task_switch_count++;								//任务交换的计数值
		#endif	
		
		OS_TASK_SW();										//进行任务调度
	}

	OS_EXIT_CRITICAL();
}

/*
****************************************************************************************
*                                     初始化TCB结构
*
* 函数名称	：SLOS_tcb_init()
* 创建日期	：2009/5/15
* 修改日期	：2009/5/15
* 说明	：初始化任务的TCB结构，在任务创建任务时调用
* 参数	：	task	:任务的执行代码地址
*			stk	:堆栈的地址
*			prio	:任务的优先级
* 返回	：无
****************************************************************************************
*/
OS_TCB * SLOS_tcb_init(void (*task)(void *pd), INT8U *name, INT8U prio)
{
	OS_TCB	*ptcb;										//临时用的TCB指针
	INT8U	i;	
	
	ptcb = SLOS_tcb_free_list;							//从TCB空闲链表中取得一个TCB结构
	if(ptcb == (OS_TCB*)0){								//检测TCB列表中取得的tcb是否为空，即检测tcb列表是否为空
		return (OS_TCB *)0;		
	}
	
	SLOS_tcb_free_list = ptcb->NextTcb;					//从tcb空闲列表取出一个tcb结构后，将tcb空闲列表的表头指针指向下一个tcb
	
	ptcb->Prio				= prio;						//在TCB中记录优先级
	ptcb->State				= OS_STATE_CREATED;			//将TCB的任务状态设为已创建
	
	#if TASK_SCHED_TIME_EN > 0
	ptcb->TimeLen			= TASK_TIME_LEN;			//在TCB中记录同级轮转的时间片间隔数
	ptcb->CurTime			= TASK_TIME_LEN;			//在TCB中记录同级轮转还剩下多少时间
	#endif	
	
	#if MIN_PRIO<= 63
    ptcb->OSTCBY         = (INT8U)(prio >> 3);			//设置x,y标志位
    ptcb->OSTCBBitY      = (INT8U)(1 << ptcb->OSTCBY);
    ptcb->OSTCBX         = (INT8U)(prio & 0x07);
    ptcb->OSTCBBitX      = (INT8U)(1 << ptcb->OSTCBX);
	#else
    ptcb->OSTCBY         = (INT8U)((prio >> 4) & 0xFF);	//设置x,y标志位
    ptcb->OSTCBBitY      = (INT16U)(1 << ptcb->OSTCBY);
    ptcb->OSTCBX         = (INT8U)(prio & 0x0F);
    ptcb->OSTCBBitX      = (INT16U)(1 << ptcb->OSTCBX);
	#endif    
	
	#if TASK_DELAY_EN > 0
	ptcb->Delay			= 0;							//在TCB中记录延时时间
	#endif

	for(i = 0;i < TASK_NAME_LEN;i++){					//复制name参数的任务名字到TCB中
		ptcb->Name[i] = *name;
		if(*name == '\0'){
			break;
		}
		name++;
	}
	ptcb->Name[TASK_NAME_LEN - 1] = '\0';				//防止溢出

	#if CPU_HOOKS_EN > 0
	OSTCBInitHook(ptcb);
	#endif

	return ptcb;	
}

/*
****************************************************************************************
*                                    开始系统的运行
*
* 函数名称	：SLOS_start()
* 创建日期	：2009/5/16
* 修改日期	：2009/5/16
* 说明		：选择最高优先级的任务，并运行。使系统进入多任务状态
* 参数		：无
* 返回		：无
****************************************************************************************
*/
void SLOS_start()
{	
	SLOS_get_heighready();
	OSTCBCur = SLOS_tcb_by_prio[SLOS_Prio_highready];		//设置当前正在运行的任务TCB指针
	OSTCBHighRdy = SLOS_tcb_by_prio[SLOS_Prio_highready];	//设置即将运行的任务TCB指针，在OSStartHighRdy()用到	
	OSStartHighRdy();										//调用硬件相关的汇编代码开始最高优先级的任务
}

/*
****************************************************************************************
*                                    时钟中断处理函数
*
* 函数名称	：OSTimeTick()
* 创建日期	：2009/5/28
* 修改日期	：2009/5/28
* 说明		：选择最高优先级的任务，并运行。使系统进入多任务状态
* 参数		：无
* 返回		：无
****************************************************************************************
*/
void OSTimeTick()
{
	#if OS_CRITICAL_METHOD == 3							//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif	

	OS_TCB *ptcb;
	#if TASK_DELAY_EN > 0								//对任务延时代码进行剪裁
	OS_TCB *ptcb2;
	#endif

	#if TASK_STATA_EN > 0
	time_tick_count++;	
	#endif
	
	ptcb = SLOS_tcb_delay_list;

	#if TASK_DELAY_EN > 0 || OS_EVENT_EN > 0			//对任务延时代码进行剪裁
	OS_ENTER_CRITICAL();
	while(ptcb != (OS_TCB *)0){							//循环遍历整个延时链表，递减每个元素的Delay进行延时计数
	
		if(ptcb->Delay == 0){							//如果delay设置的是0的话，则是永远都延时，不需唤醒
			ptcb = ptcb->NextTcb;
			continue;					
		}
			
		if(--ptcb->Delay == 0){							//如果到了延时时间，则脱离延时链表
		
			if(ptcb->LastTcb != (OS_TCB *)0){			//判断是不是链表的第一个元素
				ptcb->LastTcb->NextTcb = ptcb->NextTcb;	//如果不是第一个
			}else{
				SLOS_tcb_delay_list = ptcb->NextTcb;	//如果是第一个，要改链表头指针
			}
			if(ptcb->NextTcb != (OS_TCB *)0){			//判断是不是链表的最后一个元素
				ptcb->NextTcb->LastTcb = ptcb->LastTcb;	//如果是，就不必设置下一个元素的LastTcb指针
			}

			#if OS_EVENT_EN >0
			if(ptcb->State != OS_STATE_DEALY){
				ptcb->Request = SLOS_ERR_TIME_OUT;
			}
			#endif
			
 			ptcb2 = ptcb->NextTcb;						//下一个tcb

			slos_insert_ready(ptcb);					//插入到就绪链表
			ptcb = ptcb2;
		}else{
			ptcb = ptcb->NextTcb;						//下一个tcb	//之所以和上句的上句一样，是因为如果已经插入到就绪，那么ptcb的nexttcb就是就绪表中的next了		
		}
	}	
	#endif

	#if TASK_SCHED_TIME_EN > 0							//对同级轮转代码进行剪裁	
	if(--OSTCBCur->CurTime == 0){						//进行统计时间片轮转剩余时间的计算，如果当前任务已经用完了一个时间片
		if(OSTCBCur->NextTcb != OSTCBCur){				//如果当前任务有同优先级的其他任务		
			SLOS_tcb_by_prio[OSTCBCur->Prio] = OSTCBCur->NextTcb;			
		}
		OSTCBCur->CurTime = TASK_TIME_LEN;
	}	
	#endif

	#if CPU_HOOKS_EN > 0
	OSTimeTickHook();
	#endif

	OS_EXIT_CRITICAL();
}

/*
****************************************************************************************
*                                    时钟中断处理函数
*
* 函数名称	：OSIntEnter()
* 创建日期	：2009/7/16
* 修改日期	：2009/7/16
* 说明		：进入中断处理前调用
* 参数		：无
* 返回		：无
****************************************************************************************
*/
void OSIntEnter()
{
	#if OS_CRITICAL_METHOD == 3					//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif

	OS_ENTER_CRITICAL();
    if (OSIntNesting < 255) {
        OSIntNesting++;							//增加中断计数
    }
    OS_EXIT_CRITICAL();
}

/*
****************************************************************************************
*                                    时钟中断处理函数
*
* 函数名称	：OSIntExit()
* 创建日期	：2009/7/16
* 修改日期	：2009/7/16
* 说明		：中断处理后调用
* 参数		：无
* 返回		：无
****************************************************************************************
*/
void OSIntExit()
{
	#if OS_CRITICAL_METHOD == 3									//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif

	if (OSIntNesting > 0) {										//减少中断计数
    	OSIntNesting--;
    }
	
	if(OSIntNesting == 0){
		SLOS_get_heighready();									//找到当前最高优先级的就绪任务
		if(OSTCBCur != SLOS_tcb_by_prio[SLOS_Prio_highready]){	//只有当最高优先级任务不是当前任务的情况下才进行任务切换
			OS_ENTER_CRITICAL();
			
			#if CPU_HOOKS_EN > 0
			OSTaskSwHook();			
			#endif
			
			OSTCBHighRdy = SLOS_tcb_by_prio[SLOS_Prio_highready];

			#if TASK_STATA_EN > 0
			task_switch_count++;								//任务交换的计数值
			#endif	

			OS_EXIT_CRITICAL();
			
			OSIntCtxSw();										//进行任务调度
			
		}
	}
}


/*
****************************************************************************************
*                                 将一个tcb插入到就绪链表
*
* 函数名称	：slos_insert_ready(OS_TCB *ptcb)
* 创建日期	：2009/5/29
* 修改日期	：2009/5/29
* 说明		：将一个tcb插入到分级就绪链表
* 参数		：	ptcb:	TCB结构的指针
* 返回		：无
****************************************************************************************
*/
void slos_insert_ready(OS_TCB *ptcb)
{
	INT8U	prio;
	OS_TCB	*ptcbtem;

	prio = ptcb->Prio;							//获取要插入的任务的任务的优先级
	ptcbtem = SLOS_tcb_by_prio[prio];			//获取对应优先级的循环链表的链表头指针
	
	if(ptcbtem == (OS_TCB *)0){
		ptcb->NextTcb = ptcb;
		ptcb->LastTcb = ptcb;
		SLOS_tcb_by_prio[prio] = ptcb;		
	}else{
		ptcb->NextTcb = ptcbtem->NextTcb;		//把tcb插入到表头所指向的tcb的下一个位置，而不是指向的位置。这样可以使调度更加合理
		ptcb->LastTcb = ptcbtem;
		ptcbtem->NextTcb->LastTcb = ptcb;
		ptcbtem->NextTcb = ptcb;
	}

	ptcb->State = OS_STATE_READY;

	OSRdyGrp               |= ptcb->OSTCBBitY;	//将任务放入就绪位图
    OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;

}

/*
****************************************************************************************
*                                 将一个tcb脱离就绪链表
*
* 函数名称	：slos_out_ready(OS_TCB *ptcb)
* 创建日期	：2009/5/29
* 修改日期	：2009/5/29
* 说明		：将一个tcb脱离分级就绪链表
* 参数		：	ptcb:	TCB结构的指针
* 返回		：无
****************************************************************************************
*/
void slos_out_ready(OS_TCB *ptcb)
{
	INT8U prio;
	INT8U y;	

 	prio = ptcb->Prio;	
	
	if(ptcb->NextTcb == ptcb){					//如果当前优先级的循环链表中只有这一个tcb了
		SLOS_tcb_by_prio[prio] = (OS_TCB *)0;	//由于进行任务调度时要对SLOS_tcb_by_prio[prio]和OSTCBCur进行对比，所以这里还是要进行设置的		
		
		y            = ptcb->OSTCBY;			//清空就绪位图上的标记
    	OSRdyTbl[y] &= ~ptcb->OSTCBBitX;                      
    	if (OSRdyTbl[y] == 0) {
        	OSRdyGrp &= ~ptcb->OSTCBBitY;
    	}
	}else{
		ptcb->NextTcb->LastTcb = ptcb->LastTcb;
		ptcb->LastTcb->NextTcb = ptcb->NextTcb;

		if(SLOS_tcb_by_prio[prio] == ptcb){
			SLOS_tcb_by_prio[prio] = ptcb->NextTcb;
		}
	}

}

/*
****************************************************************************************
*                          得到当前最高优先级的就绪任务
*
* 函数名称	：SLOS_get_heighready()
* 创建日期	：2009/7/8
* 修改日期	：2009/7/8
* 说明		：得到当前最高优先级的就绪任务并将赋值SLOS_Prio_highready
* 参数		：无			
* 返回		：无
****************************************************************************************
*/
void SLOS_get_heighready()
{			   	
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
	    OS_CPU_SR  cpu_sr;
	#endif 
	
	#if MIN_PRIO <= 63								//如果总的优先级数小于64，则采用8*8的就绪位图
    INT8U   y;
    y             = OSUnMapTbl[OSRdyGrp];
	OS_ENTER_CRITICAL();
    SLOS_Prio_highready = (INT8U)((y << 3) + OSUnMapTbl[OSRdyTbl[y]]);
	#else											//如果总的优先级数大于64，则采用16*16的就绪位图
    INT8U   y;
    INT16U *ptbl;
	OS_ENTER_CRITICAL();
    if ((OSRdyGrp & 0xFF) != 0) {
        y = OSUnMapTbl[OSRdyGrp & 0xFF];
    } else {
        y = OSUnMapTbl[(OSRdyGrp >> 8) & 0xFF] + 8;
    }
    ptbl = &OSRdyTbl[y];
    if ((*ptbl & 0xFF) != 0) {
        SLOS_Prio_highready = (INT8U)((y << 4) + OSUnMapTbl[(*ptbl & 0xFF)]);
    } else {
        SLOS_Prio_highready = (INT8U)((y << 4) + OSUnMapTbl[(*ptbl >> 8) & 0xFF] + 8);
    }
	#endif
	OS_EXIT_CRITICAL();
}

/*
****************************************************************************************
*                          初始化时间控制块的等待任务位图
*
* 函数名称	：SLOS_event_waitlist_init()
* 创建日期	：2009/7/9
* 修改日期	：2009/7/9
* 说明		：初始化时间控制块的等待任务位图，全部清空为0
* 参数		：	pevent:	要进行初始化的事件控制块
* 返回		：无
****************************************************************************************
*/
#if OS_EVENT_EN	> 0
void	SLOS_event_waitlist_init(OS_EVENT *pevent)
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	#if MIN_PRIO <= 63
    INT8U  *ptbl;
	#else
    INT16U *ptbl;
	#endif
    INT8U   i;

	OS_ENTER_CRITICAL();
	pevent->OSEventGrp	= 0;
	ptbl				= &pevent->OSEventTbl[0];

	for (i = 0; i < OS_EVENT_TBL_SIZE; i++) {
        *ptbl++ = 0;
    }
	OS_EXIT_CRITICAL();
}
#endif

/*
****************************************************************************************
*                          使一个任务进入等待某事件发生状态
*
* 函数名称	：SLOS_event_task_wait()
* 创建日期	：2009/7/9
* 修改日期	：2009/8/1
* 说明		：将当前任务脱离就绪表，加入事件控制块的等待链表，进入延时链表，并设置事件控制块的等待位图
* 参数		：	pevent:	事件控制块			
* 返回		：无
****************************************************************************************
*/
#if OS_EVENT_EN	> 0
void SLOS_event_task_wait(OS_EVENT *pevent)
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	

	OS_ENTER_CRITICAL();

	slos_out_ready(OSTCBCur);						//将当前任务脱离就绪任务表
	

	OSTCBCur->ELastTcb = (OS_TCB *)0;				//将当前任务加入控制块的等待链表
	OSTCBCur->ENextTcb = pevent->SLOS_tcb_by_prio[OSTCBCur->Prio];	
	if(OSTCBCur->ENextTcb != (OS_TCB *)0){			//如果原来的等待链表已经有延时tcb了
		OSTCBCur->ENextTcb->ELastTcb = OSTCBCur;	//做成双向链表
	}
	pevent->SLOS_tcb_by_prio[OSTCBCur->Prio] = OSTCBCur;
	

	OSTCBCur->LastTcb = (OS_TCB *)0;				//为进入延时链表做准备
	OSTCBCur->NextTcb = SLOS_tcb_delay_list;		//插入到延时链表中		
	if(OSTCBCur->NextTcb != (OS_TCB *)0){			//如果原来的延时链表已经有延时tcb了
		OSTCBCur->NextTcb->LastTcb = OSTCBCur;		//做成双向链表
	}
	SLOS_tcb_delay_list = OSTCBCur;					//插入到延时链表

	OSTCBCur->Eventptr = pevent;					//修改任务的事件控制块指针，删除任务时会用到	

	pevent->OSEventGrp |= OSTCBCur->OSTCBBitY;		//将任务放入事件控制块等待位图
	pevent->OSEventTbl[OSTCBCur->OSTCBY] |= OSTCBCur->OSTCBBitX;
	
	OS_EXIT_CRITICAL();
	
}
#endif

/*
****************************************************************************************
*                          使一个任务进入就绪状态
*
* 函数名称	：SLOS_event_task_ready()
* 创建日期	：2009/7/9
* 修改日期	：2009/8/1
* 说明		：将最高优先级的等待任务清除事件控制块的等待位图，脱离延时链表，脱离事件控制块的等待链表，加入就绪表
* 参数		：	pevent:	事件控制块
*				pmsg:	要传递的消息				
*			
* 返回		：无
****************************************************************************************
*/
#if OS_EVENT_EN	> 0
void SLOS_event_task_ready(OS_EVENT *pevent, void *pmsg)
{
	#if OS_CRITICAL_METHOD == 3			//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	INT8U	hprio;
	OS_TCB	*ptcb;
	#if MIN_PRIO <= 63
	INT8U   y;
	#else
	INT16U	y;
	INT16U *ptbl;
	#endif

	OS_ENTER_CRITICAL();
	
	if(pevent->OSEventGrp == 0){		//如果已经没有等待的任务了

		#if TASK_STATA_EN > 0			//如果允许信息统计
		pevent->State = OS_MUTEX_STATE_FREE;
		#endif
	
		pevent->OSEventCnt = 0x00FF;	//设置为空，用来标志已经没有任务使用此互斥型信号量
		OS_EXIT_CRITICAL();
		return;
	}
	
	//得到最高优先级的等待任务
	#if MIN_PRIO <= 63					//如果总的优先级数小于64，则采用8*8的就绪位图    
    y             = OSUnMapTbl[pevent->OSEventGrp];
    hprio = (INT8U)((y << 3) + OSUnMapTbl[pevent->OSEventTbl[y]]);
	#else 
    if ((pevent->OSEventGrp & 0xFF) != 0) {
        y = OSUnMapTbl[pevent->OSEventGrp & 0xFF];
    } else {
        y = OSUnMapTbl[(pevent->OSEventGrp >> 8) & 0xFF] + 8;
    }
    ptbl = &pevent->OSEventTbl[y];
    if ((*ptbl & 0xFF) != 0) {
        hprio= (INT8U)((y << 4) + OSUnMapTbl[(*ptbl & 0xFF)]);
    } else {
        hprio= (INT8U)((y << 4) + OSUnMapTbl[(*ptbl >> 8) & 0xFF] + 8);
    }
	#endif	
	ptcb = pevent->SLOS_tcb_by_prio[hprio];

	

	//清除事件控制块的等待位图	
	pevent->OSEventTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX;		//清除事件控制块的等待位图
    if (pevent->OSEventTbl[ptcb->OSTCBY] == 0) {
       	pevent->OSEventGrp &= ~ptcb->OSTCBBitY;
    }	
	
	//脱离延时链表
	if(ptcb->LastTcb != (OS_TCB *)0){							//判断是不是链表的第一个元素
		ptcb->LastTcb->NextTcb = ptcb->NextTcb;					//如果不是第一个
	}else{
		SLOS_tcb_delay_list = ptcb->NextTcb;					//如果是第一个，要改链表头指针
	}
	if(ptcb->NextTcb != (OS_TCB *)0){							//判断是不是链表的最后一个元素
		ptcb->NextTcb->LastTcb = ptcb->LastTcb;					//如果是，就不必设置下一个元素的LastTcb指针
	}

	//脱离事件控制块的等待链表	
	if(ptcb->ELastTcb != (OS_TCB *)0){
		ptcb->ELastTcb->ENextTcb = ptcb->ENextTcb;
	}else{
		pevent->SLOS_tcb_by_prio[hprio] = ptcb->ENextTcb;		//如果是分级链表的第一个，则此等级链表表头指针应指向此TCB的下一个
	}
	if(ptcb->ENextTcb != (OS_TCB *)0){
		ptcb->ENextTcb->ELastTcb = ptcb->ELastTcb;
	}

	OS_EXIT_CRITICAL();

	ptcb->State = 0;											//修改任务状态标志
	ptcb->State = OS_STATE_READY;

	ptcb->Request = SLOS_ERR_NO_ERR;							//修改标志为正常返回

	#if MBOX_EN> 0
	if(pevent->OSEventType == OS_EVENT_TYPE_MBOX){				//如果是邮箱的话这传入消息的内容
		pevent->OSEventPtr = pmsg;
	}	
	#endif

	
	#if MUTEX_EN > 0	
	
	if(pevent->OSEventType == OS_EVENT_TYPE_MUTEX){						//设置事件控制块的指针，指向当前得到此信号量的TCB，以便提升优先级时得到当前占有信号量的TCB
		pevent->OSEventCnt = (INT16U)(ptcb->Prio | (ptcb->Prio << 8));	//设置事件控制块里记录的原优先级与提升后的优先级	
		pevent->OSEventPtr = (void *)ptcb;
	}	
	#endif

	//加入就绪表
	slos_insert_ready(ptcb);
	
}
#endif

/*
****************************************************************************************
*                                     获取任务的优先级
*
* 函数名称	：INT8U taskPriorityGet(INT16U tid)
* 创建日期	：2009/7/30
* 修改日期	：2009/7/31
* 说明		：获取任务的优先级
* 参数		：	tid		:任务编号		
* 返回		：任务的优先级
****************************************************************************************
*/
#if GET_SET_PRIO_EN	> 0
INT8U taskPriorityGet(INT16U tid)
{
	OS_TCB *ptcb;
	ptcb = &SLOS_tcb[tid];		//从TCB数组中得到相应的tcb
	return ptcb->TaskId;	//返回相应tcb的tid,为适应向上接口要进行转换
}
#endif

/*
****************************************************************************************
*                                     设置任务的优先级
*
* 函数名称	：INT8S taskPrioritySet(INT16U tid, INT8U newPriority)
* 创建日期	：2009/7/30
* 修改日期	：2009/7/31
* 说明		：设置任务优先级, 用于动态调整
* 参数		：	tid				:任务编号
*				newPriority		:新的优先级
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if GET_SET_PRIO_EN	> 0
INT8S taskPrioritySet(INT16U tid, INT8U newPriority)
{
	OS_TCB	*ptcb;
	INT8U	newprio;

	ptcb	= &SLOS_tcb[tid];
	newprio	= (INT8U)newPriority;
	
	if(ptcb->State == OS_STATE_READY){							//只有当任务是就绪的，才可以把它先先脱离，然后插入到就绪链表中，来调整tcb在分级就绪链表中的位置
		slos_out_ready(ptcb);									//将此任务脱离就绪任务表

		#if MIN_PRIO<= 63										//重新设置改变优先级后的任务的x，y标志位
		ptcb->OSTCBY         = (INT8U)(newprio >> 3);			//设置x,y标志位
		ptcb->OSTCBBitY      = (INT8U)(1 << ptcb->OSTCBY);
		ptcb->OSTCBX         = (INT8U)(newprio & 0x07);
		ptcb->OSTCBBitX      = (INT8U)(1 << ptcb->OSTCBX);
		#else			
		ptcb->OSTCBY         = (INT8U)((newprio >> 4) & 0xFF);	//设置x,y标志位
		ptcb->OSTCBBitY      = (INT16U)(1 << ptcb->OSTCBY);
		ptcb->OSTCBX         = (INT8U)(newprio & 0x0F);
		ptcb->OSTCBBitX      = (INT16U)(1 << ptcb->OSTCBX);
		#endif
			
		ptcb->Prio = newprio;									//改变此任务的优先级
			
		slos_insert_ready(ptcb);								//将提升优先级后的任务插入到就绪任务表中
	}else{														//如果不是就绪的，那么只需提高优先级
		#if MIN_PRIO<= 63										//重新设置改变优先级后的任务的x，y标志位
		ptcb->OSTCBY         = (INT8U)(newprio >> 3);			//设置x,y标志位
		ptcb->OSTCBBitY      = (INT8U)(1 << ptcb->OSTCBY);
		ptcb->OSTCBX         = (INT8U)(newprio & 0x07);
		ptcb->OSTCBBitX      = (INT8U)(1 << ptcb->OSTCBX);
		#else
		ptcb->OSTCBY         = (INT8U)((newprio >> 4) & 0xFF);	//设置x,y标志位
		ptcb->OSTCBBitY      = (INT16U)(1 << ptcb->OSTCBY);
		ptcb->OSTCBX         = (INT8U)(newprio & 0x0F);
		ptcb->OSTCBBitX      = (INT16U)(1 << ptcb->OSTCBX);
		#endif		
			
		ptcb->Prio = newprio;									//改变此任务的优先级
	}

	return SLOS_ERR_NO_ERR;
}
#endif


/*
****************************************************************************************
*                               查询任务的堆栈使用情况
*
* 函数名称	：void staticCheck(INT16U tid,OS_STKINFO *pstkinfo)
* 创建日期	：2009/8/5
* 修改日期	：2009/8/5
* 说明		：检测任务的堆栈使用情况
* 参数		：	tid				:任务编号
*				pstkinfo			:指向OS_STKINFO结构的指针
* 返回		：	无
****************************************************************************************
*/
#if TASK_STATA_EN > 0
void staticCheck(INT16U tid,OS_STKINFO *pstkinfo)
{	
	INT16U	stkfree;
	OS_STK	*pstk;
	OS_TCB	*ptcb;

	ptcb	= &SLOS_tcb[tid];
	
	#if OS_STK_GROWTH ==0							//堆栈是递增的
	pstk = &SLOS_stk[ptcb->TaskId][SLOS_stk_SIZE - 1];
	stkfree = 0;
	for(;pstk == (OS_STK *)0;pstk--){
		stkfree++;
	}
	#else											//堆栈是递减的
	pstk = &SLOS_stk[ptcb->TaskId][0];
	stkfree = 0;
	for(;*pstk == (OS_STK)0;pstk++){
		stkfree++;
	}
	#endif

	pstkinfo->StkFree = stkfree;
	pstkinfo->StkUsed = (INT16U)(TASK_STATIC_SIZE - stkfree);
}
#endif

/*
****************************************************************************************
*                                     任务交换的HOOK函数
*
* 函数名称	：void OSTaskSwHook (void)
* 创建日期	：2009/8/2
* 修改日期	：2009/8/2
* 说明		：为符合ucos的硬件抽象层而定义此函数
* 参数		：无
* 返回		：无
****************************************************************************************
*/
#if CPU_HOOKS_EN == 0  //因为在ucos的Os_cpu_a.s中会对此HOOK函数函数进行调用，而SLOS禁止hooks时需提供此函数							
void OSTaskSwHook ()
{

}
#endif
