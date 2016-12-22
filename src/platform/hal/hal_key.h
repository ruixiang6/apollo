#ifndef __HAL_KEY_H
#define __HAL_KEY_H

#define KEY0        HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_3)  //KEY0按键PH3
#define KEY1        HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_2)  //KEY1按键PH2
#define KEY2        HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) //KEY2按键PC13
#define WK_UP       HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)  //WKUP按键PA0

#define KEY0_PRES 	1  	//KEY0按下后返回值
#define KEY1_PRES	2	//KEY1按下后返回值
#define KEY2_PRES	3	//KEY2按下后返回值
#define WKUP_PRES   4	//WKUP按下后返回值


extern uint8_t hal_key_scan(uint8_t mode);
extern void hal_key_init(void);


#endif
