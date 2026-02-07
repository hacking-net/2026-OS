#include "kernel/scheduler.h"

#define MAX_TASKS 4

static task_fn_t tasks[MAX_TASKS];
static uint8_t task_count = 0;
static uint8_t current_task = 0;

void scheduler_init(void) {
  for (uint8_t i = 0; i < MAX_TASKS; ++i) {
    tasks[i] = 0;
  }
  task_count = 0;
  current_task = 0;
}

int scheduler_add_task(task_fn_t task) {
  if (!task || task_count >= MAX_TASKS) {
    return -1;
  }
  tasks[task_count++] = task;
  return (int)(task_count - 1);
}

void scheduler_tick(void) {
  if (task_count == 0) {
    return;
  }
  current_task = (uint8_t)((current_task + 1) % task_count);
  task_fn_t task = tasks[current_task];
  if (task) {
    task();
  }
}

uint8_t scheduler_count(void) {
  return task_count;
}

uint8_t scheduler_current(void) {
  return current_task;
}
