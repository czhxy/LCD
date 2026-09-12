#include "main.h"

static void bsp_init(void)
{
	// 使能 LCD 相关外设时钟：GPIOA(CS)/GPIOB(SCK,MOSI)/GPIOD(DC,BL)/DMA1/SPI3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
	                       RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
}
static void task_entry(void *param)
{	
	bsp_init();
	task_ui();
	task_page();
	vTaskDelete(NULL);
}
int main()
{
//	/* ---- 外设初始化 ---- */
//	BSP_USART_Init();
//	BSP_LED_Init();
		
//	SafePrintf("FreeRTOS Template\r\n");

//	/* ---- 创建 FreeRTOS 对象 ---- */
//	xUartRxQueue = xQueueCreate(8, sizeof(UartFrame_t));

//	/* ---- 创建 FreeRTOS 任务 ---- */
//	if (xUartRxQueue != NULL)
//	{
//		xTaskCreate(UARTRxTask, "UART_RX", 256, NULL, 2, NULL);
//	}

	xTaskCreate(task_entry, "task_entry", 256, NULL, 9, NULL);

	vTaskStartScheduler();
	while (1)
	{

	}
}
