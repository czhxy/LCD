#include "task_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mod_log.h"
#include <stdio.h>

/* 监控打印周期（毫秒） */
#define MONITOR_INTERVAL_MS     5000

/* vTaskList 每个任务约占 configMAX_TASK_NAME_LEN + 40 字节，
   按 MONITOR_MAX_TASKS 个任务预留容量。
   这里用静态数组而不是动态分配，避免和业务任务抢堆 */
#define MONITOR_MAX_TASKS       12
#define MONITOR_LINE_SIZE       (configMAX_TASK_NAME_LEN + 40)
#define MONITOR_BUF_SIZE        (MONITOR_MAX_TASKS * MONITOR_LINE_SIZE)

/* 表头那行格式化后的长度约 126 字节，留够余量免得被 snprintf 截断 */
#define MONITOR_INFO_SIZE       160

static char monitor_buf[MONITOR_BUF_SIZE];

static void task_monitor_entry(void *param)
{
	(void)param;
	char info[MONITOR_INFO_SIZE];
	UBaseType_t task_num;

	while (1)
	{
		task_num = uxTaskGetNumberOfTasks();

		/* 任务数超过预留容量时这一轮直接跳过，避免写越界 */
		if ((task_num + 1) * MONITOR_LINE_SIZE <= sizeof(monitor_buf))
		{
			/* vTaskList 内部靠 pvPortMalloc 申请状态数组，申请失败时
			   一个字都不会写，所以先清空，免得打印出上一轮的旧数据 */
			monitor_buf[0] = '\0';
			vTaskList(monitor_buf);

			snprintf(info, sizeof(info),
			         "\r\n=== name / state / prio / stack / num, "
			         "task num=%u, free heap=%u, min free heap=%u ===\r\n",
			         (unsigned int)task_num,
			         (unsigned int)xPortGetFreeHeapSize(),
			         (unsigned int)xPortGetMinimumEverFreeHeapSize());
			PrintStr(info);
			PrintStr(monitor_buf);
		}
		else
		{
			PrintStr("monitor: too many tasks, enlarge monitor_buf\r\n");
		}

		vTaskDelay(pdMS_TO_TICKS(MONITOR_INTERVAL_MS));
	}
}

void task_monitor(void)
{
	xTaskCreate(task_monitor_entry, "monitor", 256, NULL, 3, NULL);
}
