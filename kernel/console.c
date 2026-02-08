#include "kernel/console.h"
#include "kernel/vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint8_t console_row = 0;
static uint8_t console_col = 0;
static uint8_t console_color = 0x1F;

static void console_newline(void) {
  console_col = 0;
  if (console_row + 1 < VGA_HEIGHT) {
    console_row++;
    return;
  }
  for (uint8_t row = 1; row < VGA_HEIGHT; ++row) {
    for (uint8_t col = 0; col < VGA_WIDTH; ++col) {
      char c = vga_read_at(row, col);
      vga_putc_at(c, console_color, row - 1, col);
    }
  }
  for (uint8_t col = 0; col < VGA_WIDTH; ++col) {
    vga_putc_at(' ', console_color, VGA_HEIGHT - 1, col);
  }
}

void console_init(uint8_t color) {
  console_row = 0;
  console_col = 0;
  console_color = color;
  vga_clear(color);
}

void console_clear(void) {
  console_row = 0;
  console_col = 0;
  vga_clear(console_color);
}

void console_putc(char c) {
  if (c == '\n') {
    console_newline();
    return;
  }
  if (c == '\b') {
    if (console_col > 0) {
      console_col--;
      vga_putc_at(' ', console_color, console_row, console_col);
    }
    return;
  }
  vga_putc_at(c, console_color, console_row, console_col);
  console_col++;
  if (console_col >= VGA_WIDTH) {
    console_newline();
  }
}

void console_write(const char *text) {
  while (*text) {
    console_putc(*text++);
  }
}

void console_write_line(const char *text) {
  console_write(text);
  console_putc('\n');
}

void console_prompt(void) {
  console_write("2026> ");
}
