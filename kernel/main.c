#include "kernel/console.h"
#include "kernel/init.h"
#include "kernel/keyboard.h"
#include "kernel/vfs.h"

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

static const char *skip_spaces(const char *s) {
  while (s && *s == ' ') {
    s++;
  }
  return s;
}

static char *find_char(char *s, char target) {
  if (!s) {
    return 0;
  }
  while (*s) {
    if (*s == target) {
      return s;
    }
    s++;
  }
  return 0;
}

static void trim_trailing_spaces(char *s) {
  if (!s) {
    return;
  }
  uint8_t len = 0;
  while (s[len]) {
    len++;
  }
  while (len > 0 && s[len - 1] == ' ') {
    s[len - 1] = '\0';
    len--;
  }
}

static void handle_ls(void) {
  uint8_t count = vfs_count();
  if (count == 0) {
    console_write_line("(pusto)");
    return;
  }
  for (uint8_t i = 0; i < count; ++i) {
    const char *name = vfs_name_at(i);
    if (name) {
      console_write_line(name);
    }
  }
}

static void handle_cat(const char *arg) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: cat <plik>");
    return;
  }
  const char *data = vfs_read(arg);
  if (!data) {
    console_write_line("Brak takiego pliku");
    return;
  }
  console_write_line(data);
}

static void handle_touch(const char *arg) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: touch <plik>");
    return;
  }
  if (vfs_write(arg, "") != 0) {
    console_write_line("Nie mozna utworzyc pliku");
  }
}

static void handle_rm(const char *arg) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: rm <plik>");
    return;
  }
  if (vfs_remove(arg) != 0) {
    console_write_line("Brak takiego pliku");
  }
}

static void handle_echo(char *args) {
  char *rest = (char *)skip_spaces(args);
  if (!rest || !rest[0]) {
    console_write_line("Uzycie: echo <tekst> [> <plik>]");
    return;
  }
  char *gt = find_char(rest, '>');
  if (!gt) {
    console_write_line(rest);
    return;
  }
  *gt = '\0';
  trim_trailing_spaces(rest);
  char *name = gt + 1;
  name = (char *)skip_spaces(name);
  if (!name || !name[0]) {
    console_write_line("Uzycie: echo <tekst> > <plik>");
    return;
  }
  if (vfs_write(name, rest) != 0) {
    console_write_line("Nie mozna zapisac pliku");
  }
}

static void console_write_uint16(uint16_t value) {
  char buffer[6];
  uint8_t pos = 0;
  if (value == 0) {
    console_putc('0');
    return;
  }
  while (value > 0 && pos < 5) {
    buffer[pos++] = (char)('0' + (value % 10));
    value /= 10;
  }
  while (pos > 0) {
    console_putc(buffer[--pos]);
  }
}

static void handle_stat(const char *arg) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: stat <plik>");
    return;
  }
  int size = vfs_size(arg);
  if (size < 0) {
    console_write_line("Brak takiego pliku");
    return;
  }
  console_write("size=");
  console_write_uint16((uint16_t)size);
  console_putc('\n');
}

static void handle_df(void) {
  console_write("files=");
  console_write_uint16(vfs_count());
  console_write("/");
  console_write_uint16(vfs_capacity());
  console_putc('\n');
}

static void handle_command(const char *command) {
  if (command[0] == '\0') {
    return;
  }
  char mutable_command[COMMAND_MAX];
  uint8_t i = 0;
  for (; i + 1 < COMMAND_MAX && command[i]; ++i) {
    mutable_command[i] = command[i];
  }
  mutable_command[i] = '\0';
  char *cmd = mutable_command;
  char *args = find_char(mutable_command, ' ');
  if (args) {
    *args = '\0';
    args = (char *)skip_spaces(args + 1);
  } else {
    args = (char *)"";
  }
  if (streq(cmd, "help")) {
    console_write_line("help  clear  about  ls  cat  echo  touch  rm  stat  df");
    return;
  }
  if (streq(cmd, "clear")) {
    console_clear();
    return;
  }
  if (streq(cmd, "about")) {
    console_write_line("2026-OS kernel shell (minimal)");
    return;
  }
  if (streq(cmd, "ls")) {
    handle_ls();
    return;
  }
  if (streq(cmd, "cat")) {
    handle_cat(args);
    return;
  }
  if (streq(cmd, "touch")) {
    handle_touch(args);
    return;
  }
  if (streq(cmd, "rm")) {
    handle_rm(args);
    return;
  }
  if (streq(cmd, "echo")) {
    handle_echo(args);
    return;
  }
  if (streq(cmd, "stat")) {
    handle_stat(args);
    return;
  }
  if (streq(cmd, "df")) {
    handle_df();
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
