#ifndef __HAL_SDMMC_H
#define __HAL_SDMMC_H

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#define SD_TIMEOUT      ((uint32_t)100000000)       //超时时间
#define SD_DMA_MODE     0	                        //1：DMA模式，0：查询模式   

extern SD_HandleTypeDef        SDCARD_Handler;      //SD卡句柄
extern HAL_SD_CardInfoTypedef  SDCardInfo;          //SD卡信息结构体


uint8_t hal_sdmmc_get_info(HAL_SD_CardInfoTypedef *cardinfo);
uint8_t hal_sdmmc_read_disk(uint8_t* buf, uint32_t sector, uint8_t cnt);
uint8_t hal_sdmmc_write_disk(uint8_t *buf, uint32_t sector, uint8_t cnt);
uint8_t hal_sdmmc_init(void);

#endif
