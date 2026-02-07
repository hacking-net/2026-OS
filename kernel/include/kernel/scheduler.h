#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/types.h"

typedef void (*task_fn_t)(void);

void scheduler_init(void);
int scheduler_add_task(task_fn_t task);
void scheduler_tick(void);
uint8_t scheduler_count(void);
uint8_t scheduler_current(void);

#endif
