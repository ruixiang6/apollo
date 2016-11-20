#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_uart.h"
    

UART_HandleTypeDef	uart_debug;



void hal_uart_send_char(uint8_t uart_id, uint8_t data)
{
	if (uart_id == UART_DEBUG)
	{
		HAL_UART_Transmit(&uart_debug, (uint8_t *)&data, 1, 5); 
	}
}


void hal_uart_send_string(uint8_t uart_id, uint8_t *string, uint32_t length)
{
	if (uart_id == UART_DEBUG)
	{
		HAL_UART_Transmit(&uart_debug, string, length, 5); 
	}	
}


void hal_uart_printf(uint8_t uart_id, uint8_t *string)
{
	if (uart_id == UART_DEBUG)
	{
		HAL_UART_Transmit(&uart_debug, string, strlen(string), 5); 
	}		
}


void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if (huart->Instance == USART1)//如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
	
		GPIO_InitStruct.Pin = GPIO_PIN_9;			//PA9
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_InitStruct.Pull = GPIO_PULLUP;			//上拉
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST; 	//高速
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1; //复用为USART1
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 	//初始化PA9

		GPIO_InitStruct.Pin = GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 	//初始化PA10
	}

}


void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		__HAL_RCC_USART1_FORCE_RESET();
		__HAL_RCC_USART1_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
	}
}


void hal_uart_deinit(uint8_t uart_id)
{
	if (uart_id == UART_DEBUG)
	{
		HAL_UART_DeInit(&uart_debug);
	}
}

void hal_uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	if (uart_id == UART_DEBUG)
	{
		//UART 初始化设置
		uart_debug.Instance = USART1;					    //USART1
		uart_debug.Init.BaudRate = baud_rate;				    //波特率
		uart_debug.Init.WordLength = UART_WORDLENGTH_8B;   //字长为8位数据格式
		uart_debug.Init.StopBits = UART_STOPBITS_1;	    //一个停止位
		uart_debug.Init.Parity = UART_PARITY_NONE;		    //无奇偶校验位
		uart_debug.Init.HwFlowCtl = UART_HWCONTROL_NONE;   //无硬件流控
		uart_debug.Init.Mode = UART_MODE_TX_RX;		    //收发模式
		HAL_UART_Init(&uart_debug);					    //HAL_UART_Init()会使能UART1		
	}
}

