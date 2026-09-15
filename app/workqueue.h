#ifndef __WORKQUEUE_H
#define __WORKQUEUE_H

typedef void (*work_t)(void *param);

void workqueue_init(void);
void workqueue_run(work_t work, void *param);

#endif /*__WORKQUEUE_H*/
