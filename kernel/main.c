#include "kernel/console.h"
#include "kernel/init.h"
#include "kernel/keyboard.h"

#define COMMAND_MAX 64

static int streq(const char *a, const char *b) {
  while (*a && *b) {
    if (*a != *b) {
      return 0;
    }
    a++;
    b++;
  }
  return *a == *b;
}

static void handle_command(const char *command) {
  if (command[0] == '\0') {
    return;
  }
  if (streq(command, "help")) {
    console_write_line("help  clear  about");
    return;
  }
  if (streq(command, "clear")) {
    console_clear();
    return;
  }
  if (streq(command, "about")) {
    console_write_line("2026-OS kernel shell (minimal)");
    return;
  }
  console_write_line("Nieznana komenda");
}

void kernel_main(void) {
  console_init(0x1F);
  console_write_line("2026-OS kernel booted");

  kernel_init();
  keyboard_init();

  console_write_line("Init: ok");

  char command[COMMAND_MAX];
  uint8_t len = 0;
  console_prompt();
  for (;;) {
    char c = keyboard_getchar();
    if (c == '\n') {
      command[len] = '\0';
      console_putc('\n');
      handle_command(command);
      len = 0;
      console_prompt();
      continue;
    }
    if (c == '\b') {
      if (len > 0) {
        len--;
        console_putc('\b');
      }
      continue;
    }
    if (len + 1 < COMMAND_MAX) {
      command[len++] = c;
      console_putc(c);
    }
  }
}
