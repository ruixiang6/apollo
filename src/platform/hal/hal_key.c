#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_key.h"



uint8_t hal_key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;     //�����ɿ���־

	if (mode == 1)
		key_up = 1;    //֧������

	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || WK_UP == 1))
    {
        delay_ms(10);
        key_up = 0;
        if (KEY0 == 0)
			return KEY0_PRES;
        else if (KEY1 == 0)
			return KEY1_PRES;
        else if (KEY2 == 0)
			return KEY2_PRES;
        else if (WK_UP == 1)
			return WKUP_PRES;          
    }
	else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && WK_UP == 0)
		key_up = 1;

	return 0;   //�ް�������
}


void hal_key_init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //����GPIOAʱ��
    __HAL_RCC_GPIOC_CLK_ENABLE();           //����GPIOCʱ��
    __HAL_RCC_GPIOH_CLK_ENABLE();           //����GPIOHʱ��
    
    GPIO_Initure.Pin = GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode = GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull = GPIO_PULLDOWN;        //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_13;           //PC13
    GPIO_Initure.Mode = GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull = GPIO_PULLUP;          //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_2 | GPIO_PIN_3; //PH2,3
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
}

