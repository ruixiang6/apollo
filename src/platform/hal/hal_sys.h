#ifndef __HAL_SYS_H
#define __HAL_SYS_H


#define Write_Through() (*(__IO uint32_t *)0XE000EF9C = 1UL << 2) //Cache透写模式 和SCB->CACR|=1<<2;作用一致

typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  
typedef const int16_t sc16;  
typedef const int8_t sc8;  

typedef  int32_t  vs32;
typedef  int16_t  vs16;
typedef  int8_t   vs8;

typedef  int32_t vsc32;  
typedef  int16_t vsc16; 
typedef  int8_t vsc8;   

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  
typedef const uint16_t uc16;  
typedef const uint8_t uc8; 

typedef  uint32_t  vu32;
typedef  uint16_t vu16;
typedef  uint8_t  vu8;

typedef  uint32_t vuc32;  
typedef  uint16_t vuc16; 
typedef  uint8_t vuc8;  



extern void hal_broad_init(void);

#endif

