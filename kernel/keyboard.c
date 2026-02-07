#include "kernel/keyboard.h"
#include "kernel/io.h"

#define PS2_STATUS 0x64
#define PS2_DATA 0x60

static const char keymap[128] = {
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
  0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,  '*', 0,
  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void keyboard_init(void) {
  (void)inb(PS2_STATUS);
}

char keyboard_getchar(void) {
  for (;;) {
    if (inb(PS2_STATUS) & 0x01) {
      uint8_t scancode = inb(PS2_DATA);
      if (scancode & 0x80) {
        continue;
      }
      char c = keymap[scancode];
      if (c != 0) {
        return c;
      }
    }
  }
}
