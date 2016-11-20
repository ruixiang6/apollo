#include <ucos_ii.h>
#include <platform.h>

/*任务管理池*/
static pool_t *task_pool;
/*任务堆栈管理*/
static void* task_stk_cb[OSEL_TASK_MAX];
/*任务堆栈*/
#pragma location = ".task_stk"
__no_init static OS_STK task_stk[OSEL_TASK_MAX][OSEL_TASK_STACK_SIZE_MAX];

bool_t osel_task_init(void)
{
	task_pool = pool_create(OSEL_TASK_MAX,	sizeof(osel_task_t));
	if (task_pool == PLAT_NULL) return PLAT_FALSE;
	
	for (uint8_t i=0; i<OSEL_TASK_MAX; i++)
	{		
		task_stk_cb[i] = (void *)&task_stk[i][OSEL_TASK_STACK_SIZE_MAX];
	}
	
	return PLAT_TRUE;
}

osel_task_t *osel_task_create(OSEL_TASK_RETURN_TYPE (*taskcode)(OSEL_TASK_PARAM_TYPE),
						OSEL_TASK_PARAM_TYPE param,
						uint32_t stack_depth,
						uint16_t prio
						)
{
	osel_task_t *task;
	uint8_t i;
	
	if (taskcode == PLAT_NULL)
	{
		return PLAT_NULL;
	}
	// alloc task pool
	task = (osel_task_t *)pool_alloc(task_pool);

	if (task == PLAT_NULL)
	{
		return PLAT_NULL;
	}
	
	/* Allocate stack space */
	for (i=0; i<OSEL_TASK_MAX; i++)
	{
		if (task_stk_cb[i] != NULL)
		{
			mem_clr(task_stk_cb[i], OSEL_TASK_STACK_SIZE_MAX);
			task->stack = task_stk_cb[i];
			task_stk_cb[i] = NULL;
			break;
		}
	}
	if (i == OSEL_TASK_MAX) 
	{
		pool_free(task_pool, (void *)task);
		return PLAT_NULL;	
	}	
	task->prio = prio;	
    task->stack_size = stack_depth;
	if (task->stack_size>OSEL_TASK_STACK_SIZE_MAX)
	{
		task->stack_size = OSEL_TASK_STACK_SIZE_MAX;
	}

	/* Create task */
	if (OSTaskCreateExt(taskcode, 
					   param, 
					   (OS_STK *)task->stack + stack_depth - 1, 
					   (uint8_t)prio,
					   (uint16_t)prio,
					   (OS_STK *)task->stack,
					   stack_depth,
					   NULL,
					   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR) == OS_ERR_NONE)
    {
    	task->handle = (uintptr_t)prio;
        return task;
    }
	else
	{
		pool_free(task_pool, (void *)task);
		return PLAT_NULL;
	}
}

bool_t osel_task_delete(osel_task_t *task)
{
	uint8_t i;
    OSEL_DECL_CRITICAL();
    
    OSEL_ENTER_CRITICAL();
	if (task != PLAT_NULL)
	{
        if (pool_free(task_pool, (void *)task) == PLAT_FALSE) 
        {
            OSEL_EXIT_CRITICAL();
            return PLAT_FALSE;
        }
		/* Free stack space */
		for (i=0; i<OSEL_TASK_MAX; i++)
		{
			if (task_stk_cb[i] == NULL)
			{
				task_stk_cb[i] = task->stack;
				break;
			}
		}
		if (i == OSEL_TASK_MAX) 
		{
			return PLAT_FALSE;	
		}		
		
        if (OSTaskDel((uint8_t)(task->handle)) == OS_ERR_NONE)
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_TRUE;
		}
		else
		{
            OSEL_EXIT_CRITICAL();
			return PLAT_FALSE;
		}
	}
	else
	{
        OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}	
}

bool_t osel_task_suspend(osel_task_t *task)
{
	return (OSTaskSuspend((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_resume(osel_task_t *task)
{
	return (OSTaskResume((uint8_t)(task->handle)) == OS_ERR_NONE);
}

bool_t osel_task_query(osel_task_t *task)
{
	OS_TCB  tcb;
		
	if (task != PLAT_NULL)
	{
		OSTaskQuery(OS_PRIO_SELF, &tcb);

		task->handle = tcb.OSTCBPrio;
		task->prio = tcb.OSTCBPrio;
		task->stack = tcb.OSTCBStkBottom + tcb.OSTCBStkSize;
		task->stack_size = tcb.OSTCBStkSize*sizeof(OS_STK);

		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}


