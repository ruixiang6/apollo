#ifndef __HAL_SYS_H
#define __HAL_SYS_H


#define Write_Through() (*(__IO uint32_t *)0XE000EF9C = 1UL << 2) //Cache͸дģʽ ��SCB->CACR|=1<<2;����һ��
 

extern void hal_broad_init(void);

#endif

