#include "mod_log.h"
extern QueueHandle_t xUartQueue;
void mod_log_init(void)
{
	BSP_USART_Init();
}

void PrintStr(const char * Str)
{
	char * pvMemory = pvPortMalloc(strlen(Str)+1);
	if(pvMemory == NULL)
	{
		return;
	}
	strcpy(pvMemory,Str);
	xQueueSend(xUartQueue,&pvMemory,portMAX_DELAY);
}
