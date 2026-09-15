#include "workqueue.h"

typedef struct
{
    work_t work;
    void *param;
} work_message_t;

static QueueHandle_t work_msg_queue;

static void workqueue_entry(void *param)
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
    xTaskCreate(workqueue_entry, "workqueue", 256, NULL, 5, NULL);
}
/*
将work回调函数和真正的执行函数打包（这里当作参数）送入队列
*/
void workqueue_send(work_t work, void *param)
{
    configASSERT(work_msg_queue);
    work_message_t msg = { work, param };
    xQueueSend(work_msg_queue, &msg, portMAX_DELAY);
}



void app_work(void *param)
{
    app_job_t job = (app_job_t)param;//把真正的执行函数转为函数指针
    job();//调用真正的业务函数
}

void work_timer_cb(TimerHandle_t timer)//重任务投入队列
{
    app_job_t job = (app_job_t)pvTimerGetTimerID(timer);//获取TimerID（实际是void*类型的真正执行函数）
    workqueue_send(app_work, job);//发到workqueue_entry中，内部执行msg.work(msg.param)，也就是app_work(job)
																	//在app_work中才执行job()，也就是真正执行回调函数的地方
}

void app_timer_cb(TimerHandle_t timer)//软任务直接跑
{
    app_job_t job = (app_job_t)pvTimerGetTimerID(timer);
    job();
}
