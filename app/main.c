#include "main.h"

static void bsp_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOC|
	                       RCC_AHB1Periph_GPIOD| RCC_AHB1Periph_DMA1|RCC_AHB1Periph_DMA2|RCC_AHB1Periph_BKPSRAM , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//rtc
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

}
static void task_entry(void *param)
{	
	bsp_init();
	workqueue_init();
	task_ui();//ui处理函数
	task_page();//发送欢迎页和主页面给ui队列
	task_uart();//串口队列发送
	
//	task_timers();//led 和 rtc的另一种实现
	task_led();
	task_rtc();
	task_monitor();
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
