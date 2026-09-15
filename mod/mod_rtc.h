#ifndef __MOD_RTC_H
#define __MOD_RTC_H
#include "bsp_rtc.h"
void mod_rtc_init(void);
void mod_rtc_set_time(const rtc_date_time_t * date_time);
void mod_rtc_get_time(rtc_date_time_t * date_time);
void mod_rtc_update(void);
#endif /*__MOD_RTC_H*/
