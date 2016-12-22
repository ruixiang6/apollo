#ifndef __HAL_EEPROM_H
#define __HAL_EEPROM_H


#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
//STM32 F746������ʹ�õ���24c02�����Զ���EE_TYPEΪAT24C02
#define EE_TYPE AT24C02

					  
extern uint8_t hal_eeprom_read_onebyte(uint16_t ReadAddr);							//ָ����ַ��ȡһ���ֽ�
extern void hal_eeprom_write_onebyte(uint16_t WriteAddr, uint8_t DataToWrite);		//ָ����ַд��һ���ֽ�
extern void hal_eeprom_write_bytes(uint16_t WriteAddr, uint32_t DataToWrite, uint8_t Len);//ָ����ַ��ʼд��ָ�����ȵ�����
extern uint32_t hal_eeprom__read_bytes(uint16_t ReadAddr, uint8_t Len);					//ָ����ַ��ʼ��ȡָ����������
extern void hal_eeprom_write(uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����
extern void hal_eeprom_read(uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����
extern uint8_t hal_eeprom_check(void);  //�������

#endif
