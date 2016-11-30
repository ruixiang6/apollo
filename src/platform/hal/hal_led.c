#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_led.h"
    

void hal_led_ctrl(uint8_t n, uint8_t status)
{
	if (n == 0)
	{
		if (status == LED_OFF)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 
		}
		else if(status == LED_ON)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		}
	}
	else if (n == 1)
	{
		if (status == LED_OFF)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 
		}
		else if(status == LED_ON)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
		}
	}
}


void hal_led_toggle(uint8_t n)
{
	if (n == 0)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	}
	else if (n == 1)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	}
}

void hal_led_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟

	GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1; //PB0,1
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull = GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOB, &GPIO_Initure);     //初始化GPIOB.0和GPIOB.1

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);	//PB1置0
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);	//PB1置1 
}

