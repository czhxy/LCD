#include "workqueue.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct
{
    work_t work;
    void *param;
} work_message_t;

static QueueHandle_t work_msg_queue;

static void work_func(void *param)
{
    work_message_t msg;
    
    while (1)
    {
        xQueueReceive(work_msg_queue, &msg, portMAX_DELAY);//传递的是数据源地址
        msg.work(msg.param);//传回调函数，参数为具体的回调函数。在app_work中最终执行
    }
}

void workqueue_init(void)
{
    work_msg_queue = xQueueCreate(16, sizeof(work_message_t));
    configASSERT(work_msg_queue);
    xTaskCreate(work_func, "workqueue", 1024, NULL, 5, NULL);
}
/*
将work回调函数和真正的执行函数打包（这里当作参数）送入队列
*/
void workqueue_run(work_t work, void *param)
{
    configASSERT(work_msg_queue);
    work_message_t msg = { work, param };
    xQueueSend(work_msg_queue, &msg, portMAX_DELAY);
}
