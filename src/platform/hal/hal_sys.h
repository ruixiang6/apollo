#ifndef __HAL_SYS_H
#define __HAL_SYS_H


#define Write_Through() (*(__IO uint32_t *)0XE000EF9C = 1UL << 2) //Cache͸дģʽ ��SCB->CACR|=1<<2;����һ��

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


extern void hal_broad_init(void);

#endif

