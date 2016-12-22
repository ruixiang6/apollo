#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_i2c.h"



//产生IIC起始信号
void hal_i2c_start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA(1);	  	  
	IIC_SCL(1);
	delay_us(4);
 	IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL(0);//钳住I2C总线，准备发送或接收数据 
}	


//产生IIC停止信号
void hal_i2c_stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL(0);
	IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL(1); 
	IIC_SDA(1);//发送I2C总线结束信号
	delay_us(4);							   	
}


//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t hal_i2c_wait_ack(void)
{
	uint8_t ucErrTime = 0;
	
	SDA_IN();      //SDA设置为输入  
	IIC_SDA(1);
	delay_us(1);	   
	IIC_SCL(1);
	delay_us(1);	 

	while (READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			hal_i2c_stop();
			return 1;
		}
	}
	
	IIC_SCL(0);//时钟输出0 	   
	return 0;  
} 


//产生ACK应答
void hal_i2c_ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}


//不产生ACK应答		    
void hal_i2c_no_ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}		


//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void hal_i2c_send_byte(uint8_t txd)
{                        
    uint8_t t;
	
	SDA_OUT(); 	    
    IIC_SCL(0);//拉低时钟开始数据传输
    
    for (t = 0; t < 8; t++)
    {              
        IIC_SDA((txd & 0x80) >> 7);
        txd <<= 1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL(1);
		delay_us(2); 
		IIC_SCL(0);	
		delay_us(2);
    }	 
} 	


//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t hal_i2c_read_byte(uint8_t ack)
{
	uint8_t i, receive = 0;
	
	SDA_IN();//SDA设置为输入
	
    for (i = 0; i < 8; i++ )
	{
        IIC_SCL(0); 
        delay_us(2);
		IIC_SCL(1);
        receive <<= 1;
        if (READ_SDA)
			receive++;   
		delay_us(1); 
    }
	
    if (!ack)
        hal_i2c_no_ack();//发送nACK
    else
        hal_i2c_ack(); //发送ACK   

	return receive;
}



void hal_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOH_CLK_ENABLE();   //使能GPIOH时钟
    
    //PH4,5初始化设置
    GPIO_Initure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FAST;     //快速
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
    
    IIC_SDA(1);
    IIC_SCL(1);  
}

