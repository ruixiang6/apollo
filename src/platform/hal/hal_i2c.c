#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_i2c.h"



//����IIC��ʼ�ź�
void hal_i2c_start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA(1);	  	  
	IIC_SCL(1);
	delay_us(4);
 	IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL(0);//ǯסI2C���ߣ�׼�����ͻ�������� 
}	


//����IICֹͣ�ź�
void hal_i2c_stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL(0);
	IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL(1); 
	IIC_SDA(1);//����I2C���߽����ź�
	delay_us(4);							   	
}


//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t hal_i2c_wait_ack(void)
{
	uint8_t ucErrTime = 0;
	
	SDA_IN();      //SDA����Ϊ����  
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
	
	IIC_SCL(0);//ʱ�����0 	   
	return 0;  
} 


//����ACKӦ��
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


//������ACKӦ��		    
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


//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void hal_i2c_send_byte(uint8_t txd)
{                        
    uint8_t t;
	
	SDA_OUT(); 	    
    IIC_SCL(0);//����ʱ�ӿ�ʼ���ݴ���
    
    for (t = 0; t < 8; t++)
    {              
        IIC_SDA((txd & 0x80) >> 7);
        txd <<= 1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL(1);
		delay_us(2); 
		IIC_SCL(0);	
		delay_us(2);
    }	 
} 	


//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t hal_i2c_read_byte(uint8_t ack)
{
	uint8_t i, receive = 0;
	
	SDA_IN();//SDA����Ϊ����
	
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
        hal_i2c_no_ack();//����nACK
    else
        hal_i2c_ack(); //����ACK   

	return receive;
}



void hal_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOH_CLK_ENABLE();   //ʹ��GPIOHʱ��
    
    //PH4,5��ʼ������
    GPIO_Initure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull = GPIO_PULLUP;          //����
    GPIO_Initure.Speed = GPIO_SPEED_FAST;     //����
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
    
    IIC_SDA(1);
    IIC_SCL(1);  
}

