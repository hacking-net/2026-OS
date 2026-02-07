#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "kernel/types.h"

void interrupts_init(void);
void interrupts_enable(void);
void interrupts_disable(void);
void pic_send_eoi(uint8_t irq);

#endif
