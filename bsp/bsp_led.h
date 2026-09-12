#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f4xx.h"

void BSP_LED_Init(void);

/* LED 任务函数声明，供 main.c 中 xTaskCreate 引用 */
void LEDTask1(void * pvParameters);

#define LED1_Port GPIOC
#define LED1_Pin GPIO_Pin_0

#endif
