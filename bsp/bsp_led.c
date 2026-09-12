#include "bsp_led.h"
#include "bsp_key.h"
#include "FreeRTOS.h"
#include "task.h"

void BSP_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LED1_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(LED1_Port, &GPIO_InitStructure);
}

void LEDTask1(void * pvParameters)
{
	(void)pvParameters;
	GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_SET);  /* ³õÊ¼Ï¨Ãð */

	while (1)
	{
			GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_RESET);  /* ÁÁ */
			vTaskDelay(pdMS_TO_TICKS(500));
			GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_SET);    /* Ãð */
			vTaskDelay(pdMS_TO_TICKS(500));
	}
}
void task_led(void)
{
	xTaskCreate(LEDTask1, "led_task", 64, NULL, 10, NULL);
}

