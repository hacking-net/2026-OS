#include "kernel/vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA_BUFFER = (uint16_t *)0xB8000;

static uint16_t vga_entry(char c, uint8_t color) {
  return (uint16_t)color << 8 | (uint8_t)c;
}

void vga_putc_at(char c, uint8_t color, uint8_t row, uint8_t col) {
  VGA_BUFFER[row * VGA_WIDTH + col] = vga_entry(c, color);
}

char vga_read_at(uint8_t row, uint8_t col) {
  uint16_t entry = VGA_BUFFER[row * VGA_WIDTH + col];
  return (char)(entry & 0xFF);
}

void vga_clear(uint8_t color) {
  for (uint16_t row = 0; row < VGA_HEIGHT; ++row) {
    for (uint16_t col = 0; col < VGA_WIDTH; ++col) {
      vga_putc_at(' ', color, row, col);
    }
  }
}

void vga_write_at(const char *text, uint8_t color, uint8_t row, uint8_t col) {
  uint16_t index = row * VGA_WIDTH + col;
  while (*text && index < VGA_WIDTH * VGA_HEIGHT) {
    VGA_BUFFER[index++] = vga_entry(*text++, color);
  }
}
