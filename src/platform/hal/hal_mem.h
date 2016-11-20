#ifndef __HAL_MEM_H
#define __HAL_MEM_H


#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
#define FLASH_WAITETIME  50000          //FLASH等待超时时间

//STM32F767 FLASH 扇区的起始地址
#if defined(DUAL_BANK)
    #define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) //扇区0起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) //扇区1起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) //扇区2起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) //扇区3起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) //扇区4起始地址, 64 Kbytes 
    #define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) //扇区5起始地址, 128 Kbytes 
    #define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) //扇区6起始地址, 128 Kbytes 
    #define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) //扇区7起始地址, 128 Kbytes 
    
    #define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08080000) //扇区0起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_13    ((uint32_t)0x08084000) //扇区0起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08088000) //扇区0起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_15    ((uint32_t)0x0808C000) //扇区0起始地址, 16 Kbytes 
    #define ADDR_FLASH_SECTOR_16    ((uint32_t)0x08090000) //扇区4起始地址, 64 Kbytes 
    #define ADDR_FLASH_SECTOR_17    ((uint32_t)0x080A0000) //扇区5起始地址, 128 Kbytes 
    #define ADDR_FLASH_SECTOR_18    ((uint32_t)0x080C0000) //扇区5起始地址, 128 Kbytes 
    #define ADDR_FLASH_SECTOR_19    ((uint32_t)0x080E0000) //扇区5起始地址, 128 Kbytes 
#else
    #define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) //扇区0起始地址, 32 Kbytes  
    #define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08008000) //扇区1起始地址, 32 Kbytes  
    #define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08010000) //扇区2起始地址, 32 Kbytes  
    #define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08018000) //扇区3起始地址, 32 Kbytes  
    #define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08020000) //扇区4起始地址, 128 Kbytes  
    #define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08040000) //扇区5起始地址, 256 Kbytes  
    #define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08080000) //扇区6起始地址, 256 Kbytes  
    #define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080C0000) //扇区7起始地址, 256 Kbytes  
#endif 

#define HAL_FLASH_BASE_ADDR 	ADDR_FLASH_SECTOR_6
#define HAL_FLASH_END_ADDR		ADDR_FLASH_SECTOR_7


void hal_flash_read(uint32_t flash_ptr, uint8_t *buf, uint16_t len);
void hal_flash_write(uint32_t flash_ptr, uint8_t *buf, uint16_t len);

#endif
