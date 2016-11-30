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
	__HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��

	GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1; //PB0,1
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull = GPIO_PULLUP;          //����
	GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPIOB, &GPIO_Initure);     //��ʼ��GPIOB.0��GPIOB.1

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);	//PB1��0
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);	//PB1��1 
}

