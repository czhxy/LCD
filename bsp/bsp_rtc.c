#include "bsp_rtc.h"
#include "stm32f4xx.h"
#include <string.h>

/* 读写日历时用于校验一致性的最大重试次数。
   原实现用无上限的 do-while，一旦写入值被截断或 RTC 停止走时就会永久卡死任务 */
#define RTC_SET_RETRY_MAX   5
#define RTC_GET_RETRY_MAX   5

/* 逐字段比较：结构体可能存在填充字节，用 memcmp 比较整体会误判 */
static uint8_t rtc_datetime_equal(const rtc_data_time_t *a, const rtc_data_time_t *b)
{
	return (uint8_t)((a->year    == b->year)   &&
	                 (a->month   == b->month)  &&
	                 (a->day     == b->day)    &&
	                 (a->hour    == b->hour)   &&
	                 (a->minute  == b->minute) &&
	                 (a->second  == b->second) &&
	                 (a->weekday == b->weekday));
}

/* 范围校验：USE_FULL_ASSERT 未开启时 assert_param 是空宏，非法值会被
   RTC_BIN2BCD 静默截断后写进寄存器，读回值永远不等于设定值 */
static uint8_t rtc_datetime_is_valid(const rtc_data_time_t *dt)
{
	if (dt == NULL)                          return 0;
	if (dt->year < 2000 || dt->year > 2099)  return 0;   /* RTC_Year 只有 0~99 */
	if (dt->month  < 1  || dt->month  > 12)  return 0;
	if (dt->day    < 1  || dt->day    > 31)  return 0;
	if (dt->hour   > 23)                     return 0;
	if (dt->minute > 59)                     return 0;
	if (dt->second > 59)                     return 0;
	if (dt->weekday < 1 || dt->weekday > 7)  return 0;
	return 1;
}

void bsp_rtc_init(void)
{	
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForSynchro();

	RTC_InitTypeDef RTC_InitStructure;
	RTC_StructInit(&RTC_InitStructure);
	RTC_Init(&RTC_InitStructure);
}

static void rtc_set_time_once(const rtc_data_time_t *date_time)
{
		RTC_DateTypeDef date;
		RTC_TimeTypeDef time;
		
		RTC_DateStructInit(&date);
		RTC_TimeStructInit(&time);
		
		date.RTC_Year = date_time->year - 2000;
		date.RTC_Month = date_time->month;
		date.RTC_Date = date_time->day;
		date.RTC_WeekDay = date_time->weekday;
		time.RTC_Hours = date_time->hour;
		time.RTC_Minutes = date_time->minute;
		time.RTC_Seconds = date_time->second;
		
		RTC_SetDate(RTC_Format_BIN, &date);
		RTC_SetTime(RTC_Format_BIN, &time);
}
static void rtc_get_time_once(rtc_data_time_t *date_time)
{
		RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    
    RTC_DateStructInit(&date);
    RTC_TimeStructInit(&time);

    /* 先读时间再读日期：RTC_TR 与 RTC_DR 的影子寄存器更新可能相差一个
       RTCCLK 周期，跨日瞬间按这个顺序读到的组合不易出现新旧混搭 */
    RTC_GetTime(RTC_Format_BIN, &time);
    RTC_GetDate(RTC_Format_BIN, &date);
    
    date_time->year = 2000 + date.RTC_Year;
    date_time->month = date.RTC_Month;
    date_time->day = date.RTC_Date;
    date_time->weekday = date.RTC_WeekDay;
    date_time->hour = time.RTC_Hours;
    date_time->minute = time.RTC_Minutes;
    date_time->second = time.RTC_Seconds;
}
void bsp_rtc_set_time(const rtc_data_time_t *date_time)
{
	rtc_data_time_t rtime;
	uint8_t i;

	/* 参数非法就直接放弃：写进去的值会被截断，读回永远对不上，
	   旧实现的无上限 do-while 会在这里永久卡死 RTC 任务 */
	if (rtc_datetime_is_valid(date_time) == 0)
	{
		return;
	}

	for (i = 0; i < RTC_SET_RETRY_MAX; i++)
	{
		rtc_set_time_once(date_time);
		rtc_get_time_once(&rtime);
		if (rtc_datetime_equal(&rtime, date_time))
		{
			break;
		}
	}
}

void bsp_rtc_get_time(rtc_data_time_t *date_time)
{
	rtc_data_time_t time1;
	rtc_data_time_t time2;
	uint8_t i;

	if (date_time == NULL)
	{
		return;
	}

	/* 连续两次读到同一组值才认为数据稳定（影子寄存器更新可能正好跨过读取时刻）。
	   同样加上限，避免 RTC 异常时任务卡死 */
	rtc_get_time_once(&time1);
	for (i = 0; i < RTC_GET_RETRY_MAX; i++)
	{
		rtc_get_time_once(&time2);
		if (rtc_datetime_equal(&time1, &time2))
		{
			break;
		}
		time1 = time2;
	}

	*date_time = time1;
}
