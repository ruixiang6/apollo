#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_i2c.h"
#include "hal_pcf8574.h"
#include "hal_timer.h"



//读取PCF8574的8位IO值
//返回值:读到的数据
uint8_t hal_pcf8574_read_onebyte(void)
{				  
	uint8_t temp = 0;
	
    hal_i2c_start();    	 	   
	hal_i2c_send_byte(PCF8574_ADDR | 0X01);   //进入接收模式			   
	hal_i2c_wait_ack();	 
    temp = hal_i2c_read_byte(0);		   
    hal_i2c_stop();							//产生一个停止条件	    
    
	return temp;
}


//向PCF8574写入8位IO值  
//DataToWrite:要写入的数据
void hal_pcf8574_write_oneByte(uint8_t DataToWrite)
{				   	  	    																 
    hal_i2c_start();  
    hal_i2c_send_byte(PCF8574_ADDR|0X00);   //发送器件地址0X40,写数据 	 
	hal_i2c_wait_ack();	    										  		   
	hal_i2c_send_byte(DataToWrite);    	 	//发送字节							   
	hal_i2c_wait_ack();      
    hal_i2c_stop();							//产生一个停止条件 
	delay_ms(10);	 
}

//设置PCF8574某个IO的高低电平
//bit:要设置的IO编号,0~7
//sta:IO的状态;0或1
void hal_pcf8574_write_bit(uint8_t bit, uint8_t sta)
{
    uint8_t data;
    data = hal_pcf8574_read_onebyte(); //先读出原来的设置
    if (sta == 0)
		data &= ~(1 << bit);     
    else 
		data |= 1 << bit;
	
    hal_pcf8574_write_oneByte(data); //写入新的数据
}

//读取PCF8574的某个IO的值
//bit：要读取的IO编号,0~7
//返回值:此IO的值,0或1
uint8_t hal_pcf8574_read_bit(uint8_t bit)
{
    uint8_t data;
	
    data = hal_pcf8574_read_onebyte(); //先读取这个8位IO的值 
    if (data & (1 << bit))
		return 1;
    else 
		return 0;   
}  


uint8_t hal_pcf8574_init(void)
{
    uint8_t temp = 0;
    GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOB_CLK_ENABLE();           //使能GPIOB时钟
	
    GPIO_Initure.Pin = GPIO_PIN_12;           //PB12
    GPIO_Initure.Mode = GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull = GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);     //初始化
    
	//检查PCF8574是否在位
    hal_i2c_start();    	 	   
	hal_i2c_send_byte(PCF8574_ADDR);            //写地址			   
	temp = hal_i2c_wait_ack();		            //等待应答,通过判断是否有ACK应答,来判断PCF8574的状态
    hal_i2c_stop();					            //产生一个停止条件
    hal_pcf8574_write_oneByte(0XFF);	            //默认情况下所有IO输出高电平
    
	return temp;
}

    
