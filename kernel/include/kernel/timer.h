#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "kernel/types.h"

void timer_init(uint32_t frequency);
uint64_t timer_ticks(void);

#endif
