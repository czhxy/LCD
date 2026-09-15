#include "task_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t xUartQueue;
static void task_uart_entry(void *param)
{
	(void)param;
	xUartQueue = xQueueCreate(10,sizeof(char *));
	mod_log_init();
	while(1)
	{
		char *pMemory =NULL;
		xQueueReceive(xUartQueue,&pMemory,portMAX_DELAY);
		printf("%s",pMemory);
		vPortFree(pMemory);
	}
}

void task_uart(void)
{
	xTaskCreate(task_uart_entry, "UART", 256, NULL, 8, NULL);
}


