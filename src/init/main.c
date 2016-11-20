#include "platform.h"



#define INIT_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define INIT_TASK_PRIO				OSEL_TASK_PRIO(0)

OSEL_DECLARE_TASK(INIT_TASK, param);
osel_task_t *init_task_h;


void main(void)
{
	DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
	
	hal_broad_init();

	hal_led_init();
	
	hal_sdram_init();
	
	hal_uart_init(UART_DEBUG, 115200);
	
	memory_init(NULL);
	
	DBG_PRINTF("hello\r\n");

	osel_init();

	init_task_h = osel_task_create(INIT_TASK, 
    								NULL, 
    								INIT_TASK_STK_SIZE, 
    								INIT_TASK_PRIO);
    DBG_ASSERT(init_task_h != PLAT_NULL);

	osel_start();

	while(1);
}


OSEL_DECLARE_TASK(INIT_TASK, param)
{
	(void)param;
	bool_t res;
	
	DBG_TRACE("INIT_TASK!\r\n");

	test_init();	

    while(1)
	{		
		osel_task_delete(init_task_h);
	}
}

