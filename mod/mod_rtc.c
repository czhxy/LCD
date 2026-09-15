#include "mod_rtc.h"
#include "mod_log.h"
#include "mod_lcd.h"
#include "stm32f4xx.h"
#include <string.h>
#define RTC_TIME_INIT_MAGIC   0xA5A5
static const rtc_date_time_t default_time = {
	.year = 2026, .month = 9, .day = 15,
	.hour = 15, .minute = 50, .second = 30, .weekday = 2
};
void mod_rtc_init(void)
{	
	bsp_rtc_init();

	/* 只有备份域被复位过才写默认时间，接了 VBAT 时普通复位不会把时间打回固定值。
	   标志改到写入之后再置位：万一写时间失败，下次上电还会重试 */
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_TIME_INIT_MAGIC)
	{
		mod_rtc_set_time(&default_time);
		RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_TIME_INIT_MAGIC);
	}
}

void mod_rtc_set_time(const rtc_date_time_t * date_time)
{
	bsp_rtc_set_time(date_time);
}

void mod_rtc_get_time(rtc_date_time_t * date_time)
{
	bsp_rtc_get_time(date_time);
}

static void main_page_redraw_time(rtc_date_time_t *time)
{
    char str[6];
    char comma = (time->second % 2 == 0) ? ':' : ' ';
    snprintf(str, sizeof(str), "%02u%c%02u", time->hour, comma, time->minute);
    mod_ui_write_string(93, 11, str,mkcolor(255, 255, 255), mkcolor(5,8,13), &font24_maple_bold);
}
void mod_rtc_update(void)
{
	char buf[64]={0};
	rtc_date_time_t now_time = {0};
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
}
