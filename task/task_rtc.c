#include "task_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mod_log.h"
#include "mod_lcd.h"
extern SemaphoreHandle_t xSemUIFinsh;
void main_page_redraw_time(rtc_date_time_t *time)
{
    char str[6];
    char comma = (time->second % 2 == 0) ? ':' : ' ';
    snprintf(str, sizeof(str), "%02u%c%02u", time->hour, comma, time->minute);
    mod_ui_write_string(93, 11, str,mkcolor(255, 255, 255), mkcolor(5,8,13), &font24_maple_bold);
}
static void task_time_entry(void *param)
{
	rtc_date_time_t now_time = {0};
	char buf[64];
	mod_rtc_init();
	xSemaphoreTake(xSemUIFinsh,portMAX_DELAY);
	while(1)
	{
		mod_rtc_get_time(&now_time);
		snprintf(buf, sizeof(buf), "year=%d,mon=%d,day=%d,hour=%d,min%d,sec=%d,weekday=%d\n",
         now_time.year,
         now_time.month,
         now_time.day,
         now_time.hour,
         now_time.minute,
         now_time.second,
         now_time.weekday);

		PrintStr(buf);
		main_page_redraw_time(&now_time);
		
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void task_rtc(void)
{
	xTaskCreate(task_time_entry, "RTC", 256, NULL, 4, NULL);
}
