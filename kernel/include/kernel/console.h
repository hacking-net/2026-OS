#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include "kernel/types.h"

void console_init(uint8_t color);
void console_putc(char c);
void console_write(const char *text);
void console_write_line(const char *text);
void console_prompt(void);
void console_clear(void);

#endif
