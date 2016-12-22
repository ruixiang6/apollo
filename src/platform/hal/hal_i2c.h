#ifndef __HAL_I2C_H
#define __HAL_I2C_H


//IO方向设置
#define SDA_IN()  {GPIOH->MODER &= ~(3<<(5*2)); GPIOH->MODER |= 0<<5*2;}	//PH5输入模式
#define SDA_OUT() {GPIOH->MODER &= ~(3<<(5*2)); GPIOH->MODER |= 1<<5*2;} //PH5输出模式
//IO操作
#define IIC_SCL(n)  (n?HAL_GPIO_WritePin(GPIOH, GPIO_PIN_4, GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOH, GPIO_PIN_4, GPIO_PIN_RESET)) //SCL
#define IIC_SDA(n)  (n?HAL_GPIO_WritePin(GPIOH, GPIO_PIN_5, GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOH, GPIO_PIN_5, GPIO_PIN_RESET)) //SDA
#define READ_SDA    HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_5)  //输入SDA


extern void hal_i2c_start(void);
extern void hal_i2c_stop(void);
extern uint8_t hal_i2c_wait_ack(void);
extern void hal_i2c_ack(void);
extern void hal_i2c_no_ack(void);
extern void hal_i2c_send_byte(uint8_t txd);
extern uint8_t hal_i2c_read_byte(uint8_t ack);
extern void hal_i2c_init(void);

#endif

