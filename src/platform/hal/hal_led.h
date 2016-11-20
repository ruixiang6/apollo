#ifndef __HAL_LED_H
#define __HAL_LED_H


#define LED0		0
#define LED1		1
#define LED_OFF		0
#define LED_ON		1


extern void hal_led_ctrl(uint8_t n, uint8_t status);
extern void hal_led_init(void);

#endif
