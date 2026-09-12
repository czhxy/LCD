#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f4xx.h"

void BSP_LED_Init(void);

void LEDTask1(void * pvParameters);
void task_led(void);
#define LED1_Port GPIOC
#define LED1_Pin GPIO_Pin_0

#endif
