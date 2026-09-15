#include "mod_rtc.h"
void mod_rtc_init(void)
{
	bsp_rtc_init();
}
void mod_rtc_set_time(const rtc_data_time_t * date_time)
{
	bsp_rtc_set_time(date_time);
}
void mod_rtc_get_time(rtc_data_time_t * date_time)
{
	bsp_rtc_get_time(date_time);
}
