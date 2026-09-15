#ifndef __WORKQUEUE_H
#define __WORKQUEUE_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

typedef void (*work_t)(void *param);
typedef void (*app_job_t)(void);

void workqueue_init(void);
void workqueue_send(work_t work, void *param);
void work_timer_cb(TimerHandle_t timer);
void app_timer_cb(TimerHandle_t timer);
void app_work(void *param);
#endif /*__WORKQUEUE_H*/
