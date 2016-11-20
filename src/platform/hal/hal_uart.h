#ifndef __HAL_UART_H
#define __HAL_UART_H


#define UART_DEBUG			0


extern void hal_uart_send_char(uint8_t uart_id, uint8_t data);
extern void hal_uart_send_string(uint8_t uart_id, uint8_t *string, uint32_t length);
extern void hal_uart_printf(uint8_t uart_id, uint8_t *string);
extern void hal_uart_init(uint8_t uart_id, uint32_t baud_rate);

#endif
