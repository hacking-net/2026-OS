#include "kernel/scheduler.h"

#define MAX_TASKS 8

struct sched_slot {
  task_fn_t fn;
  uint8_t enabled;
  uint64_t runs;
};

static struct sched_slot slots[MAX_TASKS];
static uint8_t task_count = 0;
static uint8_t current_task = 0;

void scheduler_init(void) {
  for (uint8_t i = 0; i < MAX_TASKS; ++i) {
    slots[i].fn = 0;
    slots[i].enabled = 0;
    slots[i].runs = 0;
  }
  task_count = 0;
  current_task = 0;
}

int scheduler_add_task(task_fn_t task) {
  if (!task || task_count >= MAX_TASKS) {
    return -1;
  }
  slots[task_count].fn = task;
  slots[task_count].enabled = 1;
  slots[task_count].runs = 0;
  task_count++;
  return (int)(task_count - 1);
}

int scheduler_set_enabled(uint8_t task_id, uint8_t enabled) {
  if (task_id >= task_count) {
    return -1;
  }
  slots[task_id].enabled = enabled ? 1 : 0;
  return 0;
}

uint8_t scheduler_is_enabled(uint8_t task_id) {
  if (task_id >= task_count) {
    return 0;
  }
  return slots[task_id].enabled;
}

uint64_t scheduler_runs(uint8_t task_id) {
  if (task_id >= task_count) {
    return 0;
  }
  return slots[task_id].runs;
}

void scheduler_tick(void) {
  if (task_count == 0) {
    return;
  }

  for (uint8_t step = 0; step < task_count; ++step) {
    current_task = (uint8_t)((current_task + 1) % task_count);
    if (slots[current_task].enabled && slots[current_task].fn) {
      slots[current_task].runs++;
      slots[current_task].fn();
      return;
    }
  }
}

uint8_t scheduler_count(void) {
  return task_count;
}

uint8_t scheduler_current(void) {
  return current_task;
}
