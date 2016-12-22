#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

#include "platform.h"


#define MAX_TIMER_NUM   2

#define TIMER_HARDWARE_PSC_10KHZ	10800
#define TIMER_HARDWARE_ARR_5S		50000

#define TIMER_HARDWARE_MIN_US		100
#define TIMER_HARDWARE_THRESHOLD_US	(TIMER_HARDWARE_ARR_5S*TIMER_HARDWARE_MIN_US)


typedef uint32_t hs_cnt_t; /* hardware time counter */
typedef uint32_t ss_cnt_t; /* software time counter */
typedef uint64_t w_cnt_t;  /* wide time counter */

typedef struct
{
	hs_cnt_t hs_cnt;
 	ss_cnt_t ss_cnt;
} s_cnt_t;

typedef union
{
	s_cnt_t s;
	w_cnt_t w;
} hal_cnt_t;

#pragma pack(4)
typedef struct
{
	bool_t used;
	hal_cnt_t cnt;
    fpv_t func;
}timer_reg_t;
#pragma pack()


extern void delay_us(uint16_t us);
extern void delay_ms(uint16_t ms);
extern uint8_t hal_timer_alloc(uint32_t time_us, fpv_t func);
extern void hal_timer_free(uint8_t timer_id);
extern void hal_timer_init(void);

#endif
