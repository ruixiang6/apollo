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

	if (huart->Instance == USART1)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
	
		GPIO_InitStruct.Pin = GPIO_PIN_9;			//PA9
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//�����������
		GPIO_InitStruct.Pull = GPIO_PULLUP;			//����
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST; 	//����
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1; //����ΪUSART1
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 	//��ʼ��PA9

		GPIO_InitStruct.Pin = GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 	//��ʼ��PA10
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
		//UART ��ʼ������
		uart_debug.Instance = USART1;					    //USART1
		uart_debug.Init.BaudRate = baud_rate;				    //������
		uart_debug.Init.WordLength = UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
		uart_debug.Init.StopBits = UART_STOPBITS_1;	    //һ��ֹͣλ
		uart_debug.Init.Parity = UART_PARITY_NONE;		    //����żУ��λ
		uart_debug.Init.HwFlowCtl = UART_HWCONTROL_NONE;   //��Ӳ������
		uart_debug.Init.Mode = UART_MODE_TX_RX;		    //�շ�ģʽ
		HAL_UART_Init(&uart_debug);					    //HAL_UART_Init()��ʹ��UART1		
	}
}

