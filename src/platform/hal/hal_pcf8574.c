#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_i2c.h"
#include "hal_pcf8574.h"
#include "hal_timer.h"



//��ȡPCF8574��8λIOֵ
//����ֵ:����������
uint8_t hal_pcf8574_read_onebyte(void)
{				  
	uint8_t temp = 0;
	
    hal_i2c_start();    	 	   
	hal_i2c_send_byte(PCF8574_ADDR | 0X01);   //�������ģʽ			   
	hal_i2c_wait_ack();	 
    temp = hal_i2c_read_byte(0);		   
    hal_i2c_stop();							//����һ��ֹͣ����	    
    
	return temp;
}


//��PCF8574д��8λIOֵ  
//DataToWrite:Ҫд�������
void hal_pcf8574_write_oneByte(uint8_t DataToWrite)
{				   	  	    																 
    hal_i2c_start();  
    hal_i2c_send_byte(PCF8574_ADDR|0X00);   //����������ַ0X40,д���� 	 
	hal_i2c_wait_ack();	    										  		   
	hal_i2c_send_byte(DataToWrite);    	 	//�����ֽ�							   
	hal_i2c_wait_ack();      
    hal_i2c_stop();							//����һ��ֹͣ���� 
	delay_ms(10);	 
}

//����PCF8574ĳ��IO�ĸߵ͵�ƽ
//bit:Ҫ���õ�IO���,0~7
//sta:IO��״̬;0��1
void hal_pcf8574_write_bit(uint8_t bit, uint8_t sta)
{
    uint8_t data;
    data = hal_pcf8574_read_onebyte(); //�ȶ���ԭ��������
    if (sta == 0)
		data &= ~(1 << bit);     
    else 
		data |= 1 << bit;
	
    hal_pcf8574_write_oneByte(data); //д���µ�����
}

//��ȡPCF8574��ĳ��IO��ֵ
//bit��Ҫ��ȡ��IO���,0~7
//����ֵ:��IO��ֵ,0��1
uint8_t hal_pcf8574_read_bit(uint8_t bit)
{
    uint8_t data;
	
    data = hal_pcf8574_read_onebyte(); //�ȶ�ȡ���8λIO��ֵ 
    if (data & (1 << bit))
		return 1;
    else 
		return 0;   
}  


uint8_t hal_pcf8574_init(void)
{
    uint8_t temp = 0;
    GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOB_CLK_ENABLE();           //ʹ��GPIOBʱ��
	
    GPIO_Initure.Pin = GPIO_PIN_12;           //PB12
    GPIO_Initure.Mode = GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull = GPIO_PULLUP;          //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);     //��ʼ��
    
	//���PCF8574�Ƿ���λ
    hal_i2c_start();    	 	   
	hal_i2c_send_byte(PCF8574_ADDR);            //д��ַ			   
	temp = hal_i2c_wait_ack();		            //�ȴ�Ӧ��,ͨ���ж��Ƿ���ACKӦ��,���ж�PCF8574��״̬
    hal_i2c_stop();					            //����һ��ֹͣ����
    hal_pcf8574_write_oneByte(0XFF);	            //Ĭ�����������IO����ߵ�ƽ
    
	return temp;
}

    
