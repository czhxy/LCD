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
	xTaskCreate(task_uart_entry, "UART", 128, NULL, 8, NULL);
}

void PrintStr(const char * Str)
{
	char * pvMemory = pvPortMalloc(strlen(Str)+1);
	strcpy(pvMemory,Str);
	xQueueSend(xUartQueue,&pvMemory,portMAX_DELAY);
}
static void task_test_entry(void *param)
{
	while(1)
	{
		PrintStr("hello");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
void task_test(void)
{
	xTaskCreate(task_test_entry, "test", 128, NULL, 8, NULL);
}
