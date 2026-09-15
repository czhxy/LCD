#ifndef __BSP_USART_H
#define __BSP_USART_H
#include "stm32f4xx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

void BSP_USART_Init(void);
void UART_SendByte(uint8_t Byte);
void UART_SendArray(uint8_t *Array, uint16_t Length);
void UART_SendString(char *String);
void UART_SendNumber(uint32_t Number, uint8_t Length);
int fputc(int ch, FILE *f);

/* DMA 接收缓冲区大小 */
#define UART_RX_BUF_SIZE    256

/* 帧结构体：一帧完整数据 + 长度 */
typedef struct
{
    uint8_t  data[UART_RX_BUF_SIZE];
    uint16_t len;
} UartFrame_t;

/* 串口接收队列句柄 */
extern QueueHandle_t xUartRxQueue;

/* ISR 中调用的 flush 函数 */
void UART_RxFlushBufToQueue(BaseType_t *pxHigherPriorityTaskWoken, uint16_t len);

/* 串口接收任务 */
void UARTRxTask(void *pvParameters);

/* DMA 接收缓冲区（ISR 需要访问） */
extern uint8_t  rxBuf[UART_RX_BUF_SIZE];
extern uint16_t dma_last_ndtr;

#endif
