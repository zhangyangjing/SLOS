/*
*********************************************************************************************************
*                                          Sword Light
*
*                                       齐鲁软件设计大赛
*
*                               济南职业学院08软件班韶光队参赛作品
* 
* 文件名称	：OS_MBOX.C
* 创建日期	：2009/7/10
* 作者		：李佳俐
* 说明		：系统中消息邮箱相关的功能函数
*********************************************************************************************************
*/
#include "Includes.h"	//包含系统的头文件


/*
****************************************************************************************
*                                    消息邮箱的创建
*
* 函数名称	：SLOS_mbox_create()
* 创建日期	：2009/7/10
* 修改日期	：2009/7/10
* 说明		：创建一个消息邮箱
* 参数		：无
* 返回		：一个指向事件控制块的指针
****************************************************************************************
*/
#if MBOX_EN > 0
INT16U mboxCreate()
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	OS_EVENT *pevent;

	OS_ENTER_CRITICAL();
	
	pevent = SLOS_event_free_list;	//从event控制块空闲链表中获取一个event控制块
	if (pevent == (OS_EVENT *)0) {
        return (INT16U)0;	//如果event空闲链表已经为空，则直接返回
    }

	SLOS_event_free_list	= (OS_EVENT *)SLOS_event_free_list->OSEventPtr;

	pevent->OSEventType		= OS_EVENT_TYPE_MBOX;
	pevent->OSEventPtr		= (void *)0;

	#if OS_EVENT_NAME_SIZE > 1	
    pevent->OSEventName[0] = '?';			//对事件控制块的名字进行初始化
    pevent->OSEventName[1] = '\0';
	#endif

	OS_EXIT_CRITICAL();
	
	SLOS_event_waitlist_init(pevent);	//对pevent的等待任务位图进行清空初始化
	

	return pevent->EventId;	
}
#endif

/*
****************************************************************************************
*                                   发送一则消息
*
* 函数名称	：SLOS_mbox_post()
* 创建日期	：2009/5/30
* 修改日期	：2009/5/30
* 说明		：向指定邮箱发送一则消息
* 参数		：	pevent	:	指向邮箱的指针
* 			：	msg		:	要发送的消息的地址	
* 返回		：?SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if MBOX_EN > 0
INT8S mboxPost(INT16U mid,void *msg)
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 
	
	OS_EVENT	*pevent;

	OS_ENTER_CRITICAL();

	pevent = &SLOS_event[mid];

	if(pevent->OSEventGrp != 0){	//如果有任务在等待消息
		SLOS_event_task_ready(pevent,msg);	//让等待消息的任务就绪
		OS_EXIT_CRITICAL();
		
		SLOS_task_sched();
		return SLOS_ERR_NO_ERR;	
	}

	if(pevent->OSEventPtr != (void *)0){	//如果邮箱中已经有消息,出现了邮箱满溢出错误
		OS_EXIT_CRITICAL();
		return SLOS_ERR_MBOX_FUL;
	}

	pevent->OSEventPtr = msg;	//邮箱中没有邮件,则直接把消息邮件放入邮箱中
	OS_EXIT_CRITICAL();
	return SLOS_ERR_NO_ERR;

}
#endif

/*
****************************************************************************************
*                                  等待一则消息
*
* 函数名称	：SLOS_mbox_pend()
* 创建日期	：2009/5/30
* 修改日期	：2009/5/30
* 说明		：等待一则消息，指定时间等不到的话，返回错误信息
* 参数		：	pevent	:事件控制块的指针
*			：	timeout	:超时返回的时间
*			：	err		:是否执行出错
* 返回		：接收到的消息
****************************************************************************************
*/
#if MBOX_EN > 0
void *mboxPend(INT16U mid,INT16U timeout,INT8U *err)
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;	
	#endif

	OS_EVENT	*pevent;
	void *msg;	
	
	OS_ENTER_CRITICAL();

	pevent = &SLOS_event[mid];

	msg = pevent->OSEventPtr;	//获取事件控制块中的消息
	
	if(msg != (void *)0){		//如果已经有消息了
		pevent->OSEventPtr = (void *)0;	//清空消息指针以接收下一条消息
		OS_EXIT_CRITICAL();	//退出临界段
		*err = SLOS_ERR_NO_ERR;	
		return msg;				//返回消息指针		
	}

	OSTCBCur->State = OS_STATE_MBOX;	//改变状态,进入等待消息状态
	OSTCBCur->Delay = timeout;			//设置延时
	SLOS_event_task_wait(pevent);		//把当前任务挂起到等待邮箱的等待分级链表
	OS_EXIT_CRITICAL();

	SLOS_task_sched();

	OS_ENTER_CRITICAL();
	msg = pevent->OSEventPtr;	//获取事件控制块中的消息 
	
	if(msg != (void *)0){		//如果等待过程中收到了消息
		pevent->OSEventPtr = (void *)0;	//清空消息指针以接收下一条消息
		OS_EXIT_CRITICAL();	//退出临界段
		*err = SLOS_ERR_NO_ERR;	
		return msg;				//返回消息指针
	}

	
	*err = SLOS_ERR_TIME_OUT;	//超时返回了

	//清除事件控制块的等待位图
	pevent->OSEventTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX;		//清除事件控制块的等待位图
	if (pevent->OSEventTbl[OSTCBCur->OSTCBY] == 0) {
	  	pevent->OSEventGrp &= ~OSTCBCur->OSTCBBitY;
	}

	//脱离事件控制块的等待链表
	if(OSTCBCur->ELastTcb != (OS_TCB *)0){
		OSTCBCur->ELastTcb->ENextTcb = OSTCBCur->ENextTcb;
	}else{
		pevent->SLOS_tcb_by_prio[OSTCBCur->Prio] = OSTCBCur->ENextTcb;
	}
	if(OSTCBCur->ENextTcb != (OS_TCB *)0){
		OSTCBCur->ENextTcb->ELastTcb = OSTCBCur->ELastTcb;
	}	
	OS_EXIT_CRITICAL();
	return (void *)0;
}

#endif

