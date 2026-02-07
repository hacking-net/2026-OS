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

static void handle_echo(char *command) {
  char *rest = command + 4;
  rest = (char *)skip_spaces(rest);
  if (!rest || !rest[0]) {
    console_write_line("Uzycie: echo <tekst> > <plik>");
    return;
  }
  char *gt = find_char(rest, '>');
  if (!gt) {
    console_write_line("Uzycie: echo <tekst> > <plik>");
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
  char *mutable_ptr = mutable_command;
  if (streq(command, "help")) {
    console_write_line("help  clear  about  ls  cat  echo  touch  rm");
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
  if (streq(command, "ls")) {
    handle_ls();
    return;
  }
  if (streq(command, "cat")) {
    handle_cat(skip_spaces(command + 3));
    return;
  }
  if (streq(command, "touch")) {
    handle_touch(skip_spaces(command + 5));
    return;
  }
  if (streq(command, "rm")) {
    handle_rm(skip_spaces(command + 2));
    return;
  }
  if (streq(command, "echo")) {
    handle_echo(mutable_ptr);
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
