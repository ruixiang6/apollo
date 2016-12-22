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
//STM32 F746开发板使用的是24c02，所以定义EE_TYPE为AT24C02
#define EE_TYPE AT24C02

					  
extern uint8_t hal_eeprom_read_onebyte(uint16_t ReadAddr);							//指定地址读取一个字节
extern void hal_eeprom_write_onebyte(uint16_t WriteAddr, uint8_t DataToWrite);		//指定地址写入一个字节
extern void hal_eeprom_write_bytes(uint16_t WriteAddr, uint32_t DataToWrite, uint8_t Len);//指定地址开始写入指定长度的数据
extern uint32_t hal_eeprom__read_bytes(uint16_t ReadAddr, uint8_t Len);					//指定地址开始读取指定长度数据
extern void hal_eeprom_write(uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite);	//从指定地址开始写入指定长度的数据
extern void hal_eeprom_read(uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);   	//从指定地址开始读出指定长度的数据
extern uint8_t hal_eeprom_check(void);  //检查器件

#endif
