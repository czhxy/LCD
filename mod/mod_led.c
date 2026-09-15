#include "mod_led.h"
#include "bsp_led.h"
void mod_led_init(void)
{
	BSP_LED_Init();
	GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_SET);  /* ³õÊ¼Ï¨Ãð */
}
void mod_led_toggle(void)
{			
		static uint8_t count = 0;
		if(count == 0)
		{
			GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_RESET);  /* ÁÁ */
			count++;
		}
		else
		{
			GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_SET);    /* Ãð */
			count = 0;
		}
}