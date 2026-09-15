#include "task_time.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mod_log.h"
#include "mod_lcd.h"
/* 备份域复位（首次上电 / 彻底掉电）后写入的默认时间：2026-09-15 15:50:30 星期二 */
static const rtc_data_time_t default_time = {
	.year = 2026, .month = 9, .day = 15,
	.hour = 15, .minute = 50, .second = 30, .weekday = 2
};

/* 备份寄存器中标记“时间已初始化过”的魔数 */
#define RTC_TIME_INIT_MAGIC   0xA5A5
void main_page_redraw_time(rtc_data_time_t *time)
{
    char str[6];
    char comma = (time->second % 2 == 0) ? ':' : ' ';
    snprintf(str, sizeof(str), "%02u%c%02u", time->hour, comma, time->minute);
    mod_ui_write_string(93, 11, str,mkcolor(255, 255, 255), mkcolor(6, 23, 31), &font24_maple_bold);
}
static void task_time_entry(void *param)
{
	rtc_data_time_t now_time = {0};
	char buf[64];

	mod_rtc_init();

	/* 只有备份域被复位过才写默认时间，接了 VBAT 时普通复位不会把时间打回固定值。
	   标志改到写入之后再置位：万一写时间失败，下次上电还会重试 */
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_TIME_INIT_MAGIC)
	{
		mod_rtc_set_time(&default_time);
		RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_TIME_INIT_MAGIC);
	}

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
