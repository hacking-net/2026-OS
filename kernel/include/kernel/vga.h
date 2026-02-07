#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

#include "kernel/types.h"

void vga_clear(uint8_t color);
void vga_write_at(const char *text, uint8_t color, uint8_t row, uint8_t col);
void vga_putc_at(char c, uint8_t color, uint8_t row, uint8_t col);
char vga_read_at(uint8_t row, uint8_t col);

#endif
