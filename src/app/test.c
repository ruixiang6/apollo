#include <platform.h>


#define TEST_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define TEST_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(TEST_TASK, param);

osel_task_t *test_task_h;
osel_event_t *test_event_h;


void test_init(void)
{	
	/*创建 TEST 任务 */   
	test_task_h = osel_task_create(TEST_TASK, 
								NULL, 
								TEST_TASK_STK_SIZE, 
								TEST_TASK_PRIO);
	DBG_ASSERT(test_task_h != PLAT_NULL);
	test_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(test_event_h != PLAT_NULL);	
}

OSEL_DECLARE_TASK(TEST_TASK, param)
{
    (void)param;
	osel_event_res_t res;

	DBG_TRACE("TEST_TASK!\r\n");	

	while (1)
	{
		hal_led_ctrl(LED0, LED_OFF);
		hal_led_ctrl(LED1, LED_ON);
		OSTimeDly(1000);
		hal_led_ctrl(LED0, LED_ON);
		hal_led_ctrl(LED1, LED_OFF);
		OSTimeDly(1000);
	}
}
