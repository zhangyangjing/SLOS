/*
*********************************************************************************************************
*                                          Sword Light
*
*                                       齐鲁软件设计大赛
*
*                               济南职业学院08软件班韶光队参赛作品
* 
* 文件名称	：OS_MUTEX.C
* 创建日期	：2009/5/30
* 作者		：李佳俐
* 说明		：系统中各种数据结构，全局变量的定义
*********************************************************************************************************
*/
#include "Includes.h"	//包含系统的头文件


/*
****************************************************************************************
*                                     互斥信号量创建
*
* 函数名称	：INT8U semCreate(void)
* 创建日期	：2009/5/30
* 修改日期	：2009/7/31
* 说明		：创建一个互斥型信号量
* 参数		：无
* 返回		：信号量标识sid, <0: 失败
****************************************************************************************
*/
#if MUTEX_EN > 0
INT16U semCreate()
{
	#if OS_CRITICAL_METHOD == 3                      //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif
	
	OS_EVENT *pevent;

	OS_ENTER_CRITICAL();
	pevent = SLOS_event_free_list;					//从event控制块空闲链表中获取一个event控制块
	if (pevent == (OS_EVENT *)0) {
		OS_EXIT_CRITICAL();
        return SLOS_ERR_FAIL;						//如果event空闲链表已经为空，则直接返回
    }

	SLOS_event_free_list = (OS_EVENT *)SLOS_event_free_list->OSEventPtr;
	OS_EXIT_CRITICAL();

	pevent->OSEventType = OS_EVENT_TYPE_MUTEX;
	pevent->OSEventCnt 	= 0x00FF;					//当前占有此信号量的任务的优先级不可能是ff，所以可以用ff表示此信号量当前未被占用
	pevent->OSEventPtr	= (void *)0;
	
	#if OS_EVENT_NAME_SIZE > 0						//以后扩展用    
    pevent->OSEventName[0] = '\0';					//防止读取时溢出
	#endif

	#if TASK_STATA_EN > 0							//如果允许信息统计
	pevent->State = OS_MUTEX_STATE_FREE;
	#endif
	
	SLOS_event_waitlist_init(pevent);				//对pevent的等待任务位图进行清空初始化

	return pevent->EventId;							
}
#endif

/*
****************************************************************************************
*                                   等待一个互斥型信号量
*
* 函数名称	：INT8S semTake(INT16U sid)
* 创建日期	：2009/5/31
* 修改日期	：2009/5/31
* 说明		：等待一个互斥型信号量
* 参数		：	sid	:互斥型信号量的事件控制块的编号ID
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if MUTEX_EN > 0
INT8S semTake(INT16U sid)
{
	#if OS_CRITICAL_METHOD == 3							 //声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif 

	OS_EVENT	*pevent;
	INT8U 		mprio;									//mutex等待任务中最高的优先级
	OS_TCB		*ptcb;									//当前占有此信号量的tcb

	pevent = &SLOS_event[sid];

	if(pevent->OSEventType != OS_EVENT_TYPE_MUTEX){		//如果传进来的sid不是一个互斥型信号量的事件控制块
		return SLOS_ERR_FAIL;							//返回错误
	}

	OS_ENTER_CRITICAL();

	if((pevent->OSEventCnt & 0x00FF) == 0x00FF){		//如果当前mutex还没有被占用

		pevent->OSEventCnt = (INT16U)(OSTCBCur->Prio | (OSTCBCur->Prio << 8));
		pevent->OSEventPtr = OSTCBCur;

		#if TASK_STATA_EN > 0		//如果允许信息统计
		pevent->State = OS_MUTEX_STATE_TAKE;
		#endif
	
		OS_EXIT_CRITICAL();
		return SLOS_ERR_NO_ERR;

	}
	//pevent->OSEventCnt	高8位存储提权后的优先级，低8位存储原来的优先级

	mprio	= (INT8U)(pevent->OSEventCnt >> 8);			//当前正在占有mutex的任务的优先级
	ptcb	= (OS_TCB *)pevent->OSEventPtr;				//当前正在占有mutex的任务

	if(OSTCBCur->Prio < mprio){							//如果请求此信号量的任务的优先级比当前占有此信号量的优先级高，则提升当前占有此信号量的任务的优先级
		
		pevent->OSEventCnt &= 0X00FF;					//将提升到的优先级存储到event控制块中
		pevent->OSEventCnt |= OSTCBCur->Prio << 8;		//将提升到的优先级存储到event控制块中 
		
		if(ptcb->State == OS_STATE_READY){				//只有当任务是就绪的，才可以把它脱离，然后插入到就绪链表中，来调整tcb在分级链表中的位置
			slos_out_ready(ptcb);						//将占有此信号量的任务脱离就绪任务表

			#if MIN_PRIO<= 63							//重新设置改变优先级后的任务的x，y标志位
			ptcb->OSTCBY         = (INT8U)(OSTCBCur->Prio >> 3);			//设置x,y标志位
			ptcb->OSTCBBitY      = (INT8U)(1 << ptcb->OSTCBY);
			ptcb->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x07);
			ptcb->OSTCBBitX      = (INT8U)(1 << ptcb->OSTCBX);
			#else
			ptcb->OSTCBY         = (INT8U)((OSTCBCur->Prio >> 4) & 0xFF);	//设置x,y标志位
			ptcb->OSTCBBitY      = (INT16U)(1 << ptcb->OSTCBY);
			ptcb->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x0F);
			ptcb->OSTCBBitX      = (INT16U)(1 << ptcb->OSTCBX);
			#endif
				
			ptcb->Prio = OSTCBCur->Prio;									//提升占有此信号量的任务的优先级
			
			slos_insert_ready(ptcb);										//将提升优先级后的任务插入到就绪任务表中
		}else{																//如果不是就绪的，那么只需提高优先级
			#if MIN_PRIO<= 63												//重新设置改变优先级后的任务的x，y标志位
			ptcb->OSTCBY         = (INT8U)(OSTCBCur->Prio >> 3);			//设置x,y标志位
			ptcb->OSTCBBitY      = (INT8U)(1 << ptcb->OSTCBY);
			ptcb->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x07);
			ptcb->OSTCBBitX      = (INT8U)(1 << ptcb->OSTCBX);
			#else
			ptcb->OSTCBY         = (INT8U)((OSTCBCur->Prio >> 4) & 0xFF);	//设置x,y标志位
			ptcb->OSTCBBitY      = (INT16U)(1 << ptcb->OSTCBY);
			ptcb->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x0F);
			ptcb->OSTCBBitX      = (INT16U)(1 << ptcb->OSTCBX);
			#endif
			
			ptcb->Prio = OSTCBCur->Prio;									//提升占有此信号量的任务的优先级			
			
		}
		
		
	}

	OSTCBCur->Delay = EVENT_DELAY;											//设置延时	题目为什么会取消延时参数??
	OSTCBCur->State = OS_STATE_MUTEX;

	#if TASK_STATA_EN > 0													//如果允许信息统计
	pevent->State = OS_MUTEX_STATE_WAIT;
	#endif
		
	SLOS_event_task_wait(pevent);											//将当前任务加入到等待信号量状态
		
	SLOS_task_sched();														//进行任务调度

	if(OSTCBCur->Request == SLOS_ERR_TIME_OUT){								//因为超时而到达这里
	
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
		return SLOS_ERR_FAIL;
	}
	
	OS_EXIT_CRITICAL();														//正确得到了互斥型信号量
	return SLOS_ERR_NO_ERR;
	
}
#endif

/*
****************************************************************************************
*                                   释放一个互斥型信号量
*
* 函数名称	：INT8S semGive(INT16U sid)
* 创建日期	：2009/5/31
* 修改日期	：2009/5/31
* 说明		：释放一个互斥型信号量
* 参数		：	sid	:互斥型信号量的事件控制块的编号ID
* 返回		：	SLOS_ERR_NO_ERR(0)		成功
*				SLOS_ERR_FAIL(-1)		失败
****************************************************************************************
*/
#if MUTEX_EN > 0
INT8S semGive(INT16U sid)
{
	#if OS_CRITICAL_METHOD == 3												//声明关中断时保存cpu状态寄存器的变量
    OS_CPU_SR  cpu_sr;
	#endif	
	OS_EVENT	*pevent;
	
	pevent = &SLOS_event[sid];
	
	if(pevent->OSEventType != OS_EVENT_TYPE_MUTEX){							//如果传进来的sid不是一个互斥型信号量的事件控制块
		return SLOS_ERR_FAIL;												//返回错误
	}

	OS_ENTER_CRITICAL();

	if((pevent->OSEventCnt & 0x00FF) != (pevent->OSEventCnt >> 8)){			//如果任务的优先级被提高了，则应该降到原来的优先级
		slos_out_ready(OSTCBCur);											//脱离就绪链表
		
		OSTCBCur->Prio = (INT8U)(pevent->OSEventCnt & 0x00FF);				//降低到原来的优先级
		
		#if MIN_PRIO<= 63													//重新设置改变优先级后的任务的x，y标志位
	    OSTCBCur->OSTCBY         = (INT8U)(OSTCBCur->Prio >> 3);			//设置x,y标志位
	    OSTCBCur->OSTCBBitY      = (INT8U)(1 << OSTCBCur->OSTCBY);
	    OSTCBCur->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x07);
	    OSTCBCur->OSTCBBitX      = (INT8U)(1 << OSTCBCur->OSTCBX);
		#else
	    OSTCBCur->OSTCBY         = (INT8U)((OSTCBCur->Prio >> 4) & 0xFF);	//设置x,y标志位
	    OSTCBCur->OSTCBBitY      = (INT16U)(1 << OSTCBCur->OSTCBY);
	    OSTCBCur->OSTCBX         = (INT8U)(OSTCBCur->Prio & 0x0F);
	    OSTCBCur->OSTCBBitX      = (INT16U)(1 << OSTCBCur->OSTCBX);
		#endif

		slos_insert_ready(OSTCBCur);										//重新插入就绪链表
	}

	SLOS_event_task_ready(pevent,(void *)0);								//使一个新的等待的任务就绪

	SLOS_task_sched();														//进行任务调度

	OS_EXIT_CRITICAL();

	return SLOS_ERR_NO_ERR;
	
}
#endif
