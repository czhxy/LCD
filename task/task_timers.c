#include "task_timers.h"
#include "FreeRTOS.h"
#include "workqueue.h"
#include "timers.h"
#include "mod_led.h"
#include "mod_rtc.h"
/*定时任务执行*/
#define MILLISECONDS(x) (x)
#define SECONDS(x)      MILLISECONDS((x) * 1000)
#define MINUTES(x)      SECONDS((x) * 60)
#define HOURS(x)        MINUTES((x) * 60)
#define DAYS(x)          HOURS((x) * 24)

#define LED_TOGGLE_INTERVAL          MILLISECONDS(500)
#define TIME_UPDATE_INTERVAL        SECONDS(1)

static TimerHandle_t led_update_timer;
static TimerHandle_t time_update_timer;

static void task_timers_init(void)
{		
		mod_rtc_init();
		mod_led_init();
	
		time_update_timer=xTimerCreate("time_update", pdMS_TO_TICKS(TIME_UPDATE_INTERVAL), pdTRUE, mod_rtc_update, app_timer_cb);
		led_update_timer=xTimerCreate("led_toggle", pdMS_TO_TICKS(LED_TOGGLE_INTERVAL), pdTRUE, mod_led_toggle, work_timer_cb);
	
}
void task_timers(void)
{
	task_timers_init();
	//先更新一遍
	workqueue_send(app_work, mod_rtc_update);
	workqueue_send(app_work, mod_led_toggle);
	
	//开启定时器
	xTimerStart(time_update_timer, 0);
	xTimerStart(led_update_timer, 0);
}
