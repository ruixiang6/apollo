#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_led.h"
#include "hal_timer.h"
#include "hal_uart.h"

#include "debug.h"


TIM_HandleTypeDef timer3;
TIM_HandleTypeDef timer4;

static timer_reg_t timer_reg[MAX_TIMER_NUM];




void delay_us(uint16_t us)
{		
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;    	 

	ticks = us * 216; 
	told = SysTick->VAL;
	
	while (1)
	{
		tnow = SysTick->VAL;	
		if (tnow != told)
		{	    
			if (tnow < told)
				tcnt += told - tnow;
			else 
				tcnt += reload - tnow + told;	    
			told = tnow;

			if (tcnt >= ticks)
				break;
		}  
	}
}


void delay_ms(uint16_t ms)
{
	uint16_t i;
	
	for (i = 0; i < ms; i++) 
		delay_us(1000);
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM3_IRQn, 1, 3);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //����ITM3�ж�   
	}
    else if (htim->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM4_IRQn, 1, 4);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�4
		HAL_NVIC_EnableIRQ(TIM4_IRQn);          //����ITM3�ж�   
	}	
}


static void timer_create(TIM_HandleTypeDef *htim, uint16_t arr, uint16_t psc)
{
	if (htim == (TIM_HandleTypeDef *)&timer3)
		htim->Instance = TIM3;
	else if (htim == (TIM_HandleTypeDef *)&timer4)
		htim->Instance = TIM4;
	else
		DBG_ERROR("timer_create error1\n");
	
	htim->Init.Period = arr;
	htim->Init.Prescaler = psc;
	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(htim) != HAL_OK)
	{
		DBG_ERROR("timer_create error2\n");
	}
	
	return;
}


static void timer_start(TIM_HandleTypeDef *htim)
{
	__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
	
	if (HAL_TIM_Base_Start_IT(htim) != HAL_OK)
	{
		DBG_ERROR("timer_start error\n");
	}

	return;
}


static void timer_stop(TIM_HandleTypeDef *htim)
{
	if (HAL_TIM_Base_Stop_IT(htim) != HAL_OK)
	{
		DBG_ERROR("timer_stop error\n");
	}

	return;
}


void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&timer3);
}


void TIM4_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&timer4);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	int ret;
	uint16_t arr;
	static uint8_t status = 0;
	
	if (htim == (TIM_HandleTypeDef *)&timer3) 
	{
		//hal_led_toggle(LED0);
		//hal_uart_send_char(UART_DEBUG, 'c');

		if (timer_reg[0].used)
		{
			if (timer_reg[0].cnt.s.ss_cnt)
			{
				timer_create(&timer3, TIMER_HARDWARE_ARR_5S-1, TIMER_HARDWARE_PSC_10KHZ-1);
				timer_reg[0].cnt.s.ss_cnt--;			
			}
			else 
			{
				if (timer_reg[0].cnt.s.hs_cnt)
				{
					if (timer_reg[0].cnt.s.hs_cnt <= TIMER_HARDWARE_MIN_US)
						arr = 1;
					else
						arr = timer_reg[0].cnt.s.hs_cnt / TIMER_HARDWARE_MIN_US;

					timer_create(&timer3, arr-1, TIMER_HARDWARE_PSC_10KHZ-1);
					timer_reg[0].cnt.s.hs_cnt = 0;
				}
				else
				{
					timer_reg[0].used = PLAT_FALSE;
					if (timer_reg[0].func)
					{
						timer_reg[0].func();
					}			
					
					return;
				}
			}
			
			timer_start(&timer3);
		}
	}
	else if (htim == (TIM_HandleTypeDef *)&timer4) 
	{
		//hal_led_toggle(LED0);
		//hal_uart_send_char(UART_DEBUG, 'c');

		if (timer_reg[1].used)
		{
			if (timer_reg[1].cnt.s.ss_cnt)
			{
				timer_create(&timer4, TIMER_HARDWARE_ARR_5S-1, TIMER_HARDWARE_PSC_10KHZ-1);
				timer_reg[1].cnt.s.ss_cnt--;			
			}
			else 
			{
				if (timer_reg[1].cnt.s.hs_cnt)
				{
					if (timer_reg[1].cnt.s.hs_cnt <= TIMER_HARDWARE_MIN_US)
						arr = 1;
					else
						arr = timer_reg[1].cnt.s.hs_cnt / TIMER_HARDWARE_MIN_US;

					timer_create(&timer4, arr-1, TIMER_HARDWARE_PSC_10KHZ-1);
					timer_reg[1].cnt.s.hs_cnt = 0;
				}
				else
				{
					timer_reg[1].used = PLAT_FALSE;
					if (timer_reg[1].func)
					{
						timer_reg[1].func();
					}			
					
					return;
				}
			}
			
			timer_start(&timer4);
		}
	}
}


uint8_t hal_timer_alloc(uint32_t time_us, fpv_t func)
{
	uint8_t id;
	uint16_t arr;

	if (time_us == 0 || func == PLAT_NULL)
	{
		return PLAT_FALSE;
	}
	
	for (id = 0; id < MAX_TIMER_NUM; id++)
	{
		if (timer_reg[id].used == PLAT_FALSE) break;		
	}
	
	if (id == MAX_TIMER_NUM) return PLAT_FALSE;		

	timer_reg[id].used = PLAT_TRUE;
	timer_reg[id].cnt.s.ss_cnt = time_us / TIMER_HARDWARE_THRESHOLD_US;
	timer_reg[id].cnt.s.hs_cnt = time_us % TIMER_HARDWARE_THRESHOLD_US;
	timer_reg[id].func = func;
	
	if (id == 0)
	{
		if (timer_reg[id].cnt.s.ss_cnt == 0)
		{
			if (timer_reg[id].cnt.s.hs_cnt <= TIMER_HARDWARE_MIN_US)
				arr = 1;
			else
				arr = timer_reg[id].cnt.s.hs_cnt / TIMER_HARDWARE_MIN_US;
						
			timer_create(&timer3, arr-1, TIMER_HARDWARE_PSC_10KHZ-1);
			timer_reg[id].cnt.s.hs_cnt = 0;
		}
		else
		{			
			timer_create(&timer3, TIMER_HARDWARE_ARR_5S-1, TIMER_HARDWARE_PSC_10KHZ-1);
			timer_reg[id].cnt.s.ss_cnt--;
		}

		timer_start(&timer3);
	}
	else if (id == 1)
	{
		if (timer_reg[id].cnt.s.ss_cnt == 0)
		{
			if (timer_reg[id].cnt.s.hs_cnt <= TIMER_HARDWARE_MIN_US)
				arr = 1;
			else
				arr = timer_reg[id].cnt.s.hs_cnt / TIMER_HARDWARE_MIN_US;
						
			timer_create(&timer4, arr-1, TIMER_HARDWARE_PSC_10KHZ-1);
			timer_reg[id].cnt.s.hs_cnt = 0;
		}
		else
		{			
			timer_create(&timer4, TIMER_HARDWARE_ARR_5S-1, TIMER_HARDWARE_PSC_10KHZ-1);
			timer_reg[id].cnt.s.ss_cnt--;
		}

		timer_start(&timer4);
	}	

	else
	{
		DBG_ERROR("hal_timer_alloc error\n");
		return PLAT_FALSE;
	}
	
	return id+1;
}


void hal_timer_free(uint8_t timer_id)
{
	uint8_t id = timer_id - 1;
	
	if (timer_reg[id].used == PLAT_FALSE)
	{
		return;		
	}
	
	if (id == 0)
	{
		timer_stop(&timer3);

		timer_reg[id].used = PLAT_FALSE;		
		timer_reg[id].func = PLAT_NULL;
	}
	else if (id == 1)
	{
		timer_stop(&timer4);

		timer_reg[id].used = PLAT_FALSE;		
		timer_reg[id].func = PLAT_NULL;
	}
	
	return;	
}


void hal_timer_init(void)
{
	mem_clr(timer_reg, sizeof(timer_reg));
}

