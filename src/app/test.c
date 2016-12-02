#include <platform.h>


#define TEST_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define TEST_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(TEST_TASK, param);

osel_task_t *test_task_h;
osel_event_t *test_event_h;

static uint16_t test_timer_id = 0;

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

void test_timeout_cb(void)
{
	DBG_TRACE("test_timeout_cb %d\r\n", test_timer_id);

	hal_timer_free(test_timer_id);
	
	test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);
}


OSEL_DECLARE_TASK(TEST_TASK, param)
{
    (void)param;
	osel_event_res_t res;
	
	uint8_t x = 0;
	uint8_t lcd_id[12];


	DBG_TRACE("TEST_TASK!\r\n");

	//test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);

	POINT_COLOR = RED; 
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);//将LCD ID打印到lcd_id数组。	

    while (1)
    {
        switch (x) {
		case 0:
			LCD_Clear(WHITE);
			break;
		case 1:
			LCD_Clear(BLACK);
			break;
		case 2:
			LCD_Clear(BLUE);
			break;
		case 3:
			LCD_Clear(RED);
			break;
		case 4:
			LCD_Clear(MAGENTA);
			break;
		case 5:
			LCD_Clear(GREEN);
			break;
		case 6:
			LCD_Clear(CYAN);
			break; 
		case 7:
			LCD_Clear(YELLOW);
			break;
		case 8:
			LCD_Clear(BRRED);
			break;
		case 9:
			LCD_Clear(GRAY);
			break;
		case 10:
			LCD_Clear(LGRAY);
			break;
		case 11:
			LCD_Clear(BROWN);
			break;
		}
		
		POINT_COLOR=RED;
		
		LCD_ShowString(10, 40, 260, 32, 32, "Apollo STM32F4/F7"); 	
		LCD_ShowString(10, 80, 240, 24, 24, "LTDC TEST");
		LCD_ShowString(10, 110, 240, 16, 16, "ATOM@ALIENTEK");
 		LCD_ShowString(10, 130, 240, 16, 16, lcd_id);		//显示LCD ID	      					 
		LCD_ShowString(10, 150, 240, 12, 12, "2016/7/12");	
		
	    x++;
		
		if (x == 12)
			x = 0;
		
		hal_led_toggle(LED0);
		hal_led_toggle(LED1);
		
		OSTimeDly(2000);	
	}

	/*
	while (1)
	{
		//hal_led_ctrl(LED0, LED_OFF);
		//hal_led_ctrl(LED1, LED_ON);
		OSTimeDly(1000);
		//hal_led_ctrl(LED0, LED_ON);
		//hal_led_ctrl(LED1, LED_OFF);
		OSTimeDly(1000);
	}
	*/
	
}
