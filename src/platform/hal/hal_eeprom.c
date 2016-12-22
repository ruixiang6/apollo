#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_i2c.h"
#include "hal_eeprom.h"



uint8_t hal_eeprom_read_onebyte(uint16_t ReadAddr)
{				  
	uint8_t temp = 0;
	
    hal_i2c_start();  

	if (EE_TYPE > AT24C16)
	{
		hal_i2c_send_byte(0XA0);	   //����д����
		hal_i2c_wait_ack();
		hal_i2c_send_byte(ReadAddr >> 8);//���͸ߵ�ַ	    
	}
	else 
		hal_i2c_send_byte(0XA0 + ((ReadAddr/256) << 1));   //����������ַ0XA0,д���� 	   

	hal_i2c_wait_ack(); 
    hal_i2c_send_byte(ReadAddr % 256);   //���͵͵�ַ
	hal_i2c_wait_ack();	    
	hal_i2c_start();  	 	   
	hal_i2c_send_byte(0XA1);           //�������ģʽ			   
	hal_i2c_wait_ack();
	
    temp = hal_i2c_read_byte(0);		   
    hal_i2c_stop();//����һ��ֹͣ����	    

	return temp;
}


//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void hal_eeprom_write_onebyte(uint16_t WriteAddr, uint8_t DataToWrite)
{				   	  	    																 
    hal_i2c_start();  
	
	if (EE_TYPE > AT24C16)
	{
		hal_i2c_send_byte(0XA0);	    //����д����
		hal_i2c_wait_ack();
		hal_i2c_send_byte(WriteAddr >> 8);//���͸ߵ�ַ	  
	}
	else 
		hal_i2c_send_byte(0XA0 + ((WriteAddr/256) << 1));   //����������ַ0XA0,д���� 	 

	hal_i2c_wait_ack();	   
    hal_i2c_send_byte(WriteAddr % 256);   //���͵͵�ַ
	hal_i2c_wait_ack(); 	 										  		   
	hal_i2c_send_byte(DataToWrite);     //�����ֽ�							   
	hal_i2c_wait_ack();  		    	   
    hal_i2c_stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}


//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void hal_eeprom_write_bytes(uint16_t WriteAddr, uint32_t DataToWrite, uint8_t Len)
{  	
	uint8_t t;
	
	for (t = 0; t < Len; t++)
	{
		hal_eeprom_write_onebyte(WriteAddr+t, (DataToWrite >> (8*t)) & 0xff);
	}												    
}


//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
uint32_t hal_eeprom_read_bytes(uint16_t ReadAddr, uint8_t Len)
{  	
	uint8_t t;
	uint32_t temp = 0;
	
	for (t = 0; t < Len; t++)
	{
		temp <<= 8;
		temp += hal_eeprom_read_onebyte(ReadAddr + Len - t - 1); 	 				   
	}
	
	return temp;												    
}


//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
uint8_t hal_eeprom_check(void)
{
	uint8_t temp;
	
	temp = hal_eeprom_read_onebyte(255);//����ÿ�ο�����дAT24CXX			   
	if (temp == 0X55)
		return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		hal_eeprom_write_onebyte(255, 0X55);
	    temp = hal_eeprom_read_onebyte(255);	  
		if (temp == 0X55)
			return 0;
	}
	
	return 1;											  
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void hal_eeprom_read(uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead)
{
	while (NumToRead)
	{
		*pBuffer++ = hal_eeprom_read_onebyte(ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void hal_eeprom_write(uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite)
{
	while (NumToWrite--)
	{
		hal_eeprom_write_onebyte(WriteAddr, *pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}
