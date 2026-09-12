#include "main.h"

static void bsp_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOC|
	                       RCC_AHB1Periph_GPIOD| RCC_AHB1Periph_DMA1|RCC_AHB1Periph_DMA2 , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	BSP_USART_Init();
	BSP_LED_Init();
}
static void task_entry(void *param)
{	
	bsp_init();
	
	task_ui();
	task_page();
	task_led();
	task_uart();
	vTaskDelete(NULL);
}
int main()
{
	xTaskCreate(task_entry, "task_entry", 256, NULL, 9, NULL);

	vTaskStartScheduler();
	while (1)
	{

	}
}
