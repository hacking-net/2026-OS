#include "kernel/console.h"
#include "kernel/init.h"
#include "kernel/keyboard.h"
#include "kernel/vfs.h"

#define COMMAND_MAX 64
#define PATH_MAX 64

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
  uint8_t count = vfs_list_count(vfs_root());
  if (count == 0) {
    console_write_line("(pusto)");
    return;
  }
  for (uint8_t i = 0; i < count; ++i) {
    int node = vfs_list_at(vfs_root(), i);
    const char *name = vfs_name(node);
    if (name) {
      if (vfs_is_dir(node)) {
        console_write("[");
        console_write(name);
        console_write_line("]");
      } else {
        console_write_line(name);
      }
    }
  }
}

static void handle_cat(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: cat <plik>");
    return;
  }
  char name[PATH_MAX];
  int parent = vfs_resolve_parent(arg, current_dir, name, PATH_MAX);
  if (parent < 0) {
    console_write_line("Brak takiego pliku");
    return;
  }
  const char *data = vfs_read_at(parent, name);
  if (!data) {
    console_write_line("Brak takiego pliku");
    return;
  }
  console_write_line(data);
}

static void handle_touch(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: touch <plik>");
    return;
  }
  char name[PATH_MAX];
  int parent = vfs_resolve_parent(arg, current_dir, name, PATH_MAX);
  if (parent < 0) {
    console_write_line("Nie mozna utworzyc pliku");
    return;
  }
  if (vfs_write_at(parent, name, "") != 0) {
    console_write_line("Nie mozna utworzyc pliku");
  }
}

static void handle_rm(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: rm <plik>");
    return;
  }
  char name[PATH_MAX];
  int parent = vfs_resolve_parent(arg, current_dir, name, PATH_MAX);
  if (parent < 0) {
    console_write_line("Brak takiego pliku");
    return;
  }
  if (vfs_remove_at(parent, name) != 0) {
    console_write_line("Brak takiego pliku");
  }
}

static void handle_echo(char *args, int current_dir) {
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
  char filename[PATH_MAX];
  int parent = vfs_resolve_parent(name, current_dir, filename, PATH_MAX);
  if (parent < 0) {
    console_write_line("Nie mozna zapisac pliku");
    return;
  }
  if (vfs_write_at(parent, filename, rest) != 0) {
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

static void handle_stat(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: stat <plik>");
    return;
  }
  int node = vfs_resolve(arg, current_dir);
  if (node < 0) {
    console_write_line("Brak takiego pliku");
    return;
  }
  if (vfs_is_dir(node)) {
    console_write_line("To jest katalog");
    return;
  }
  int size = vfs_node_size(node);
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

static void handle_pwd(int current_dir) {
  if (current_dir == vfs_root()) {
    console_write_line("/");
    return;
  }
  int stack[8];
  uint8_t depth = 0;
  int node = current_dir;
  while (node >= 0 && node != vfs_root() && depth < 8) {
    stack[depth++] = node;
    node = vfs_parent(node);
  }
  console_putc('/');
  while (depth > 0) {
    const char *name = vfs_name(stack[--depth]);
    if (name) {
      console_write(name);
      if (depth > 0) {
        console_putc('/');
      }
    }
  }
  console_putc('\n');
}

static int handle_cd(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    return vfs_root();
  }
  int target = vfs_resolve(arg, current_dir);
  if (target < 0 || !vfs_is_dir(target)) {
    console_write_line("Brak takiego katalogu");
    return current_dir;
  }
  return target;
}

static void handle_ls_path(const char *arg, int current_dir) {
  int dir = current_dir;
  if (arg && arg[0]) {
    int target = vfs_resolve(arg, current_dir);
    if (target < 0 || !vfs_is_dir(target)) {
      console_write_line("Brak takiego katalogu");
      return;
    }
    dir = target;
  }
  if (!vfs_is_dir(dir)) {
    console_write_line("Brak takiego katalogu");
    return;
  }
  uint8_t count = vfs_list_count(dir);
  if (count == 0) {
    console_write_line("(pusto)");
    return;
  }
  for (uint8_t i = 0; i < count; ++i) {
    int node = vfs_list_at(dir, i);
    if (node < 0) {
      continue;
    }
    const char *name = vfs_name(node);
    if (name) {
      if (vfs_is_dir(node)) {
        console_write("[");
        console_write(name);
        console_write_line("]");
      } else {
        console_write_line(name);
      }
    }
  }
}

static void handle_mkdir(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: mkdir <katalog>");
    return;
  }
  char name[PATH_MAX];
  int parent = vfs_resolve_parent(arg, current_dir, name, PATH_MAX);
  if (parent < 0) {
    console_write_line("Nie mozna utworzyc katalogu");
    return;
  }
  if (vfs_mkdir_at(parent, name) != 0) {
    console_write_line("Nie mozna utworzyc katalogu");
  }
}

static void handle_rmdir(const char *arg, int current_dir) {
  if (!arg || !arg[0]) {
    console_write_line("Uzycie: rmdir <katalog>");
    return;
  }
  char name[PATH_MAX];
  int parent = vfs_resolve_parent(arg, current_dir, name, PATH_MAX);
  if (parent < 0) {
    console_write_line("Nie mozna usunac katalogu");
    return;
  }
  if (vfs_rmdir_at(parent, name) != 0) {
    console_write_line("Nie mozna usunac katalogu");
  }
}

static void handle_command(const char *command, int *current_dir) {
  if (command[0] == '\0') {
    return;
  }
  if (!vfs_is_dir(*current_dir)) {
    *current_dir = vfs_root();
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
    console_write_line("pwd  cd  mkdir  rmdir");
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
    handle_ls_path(args, *current_dir);
    return;
  }
  if (streq(cmd, "cat")) {
    handle_cat(args, *current_dir);
    return;
  }
  if (streq(cmd, "touch")) {
    handle_touch(args, *current_dir);
    return;
  }
  if (streq(cmd, "rm")) {
    handle_rm(args, *current_dir);
    return;
  }
  if (streq(cmd, "echo")) {
    handle_echo(args, *current_dir);
    return;
  }
  if (streq(cmd, "stat")) {
    handle_stat(args, *current_dir);
    return;
  }
  if (streq(cmd, "df")) {
    handle_df();
    return;
  }
  if (streq(cmd, "pwd")) {
    handle_pwd(*current_dir);
    return;
  }
  if (streq(cmd, "cd")) {
    *current_dir = handle_cd(args, *current_dir);
    return;
  }
  if (streq(cmd, "mkdir")) {
    handle_mkdir(args, *current_dir);
    return;
  }
  if (streq(cmd, "rmdir")) {
    handle_rmdir(args, *current_dir);
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
  int current_dir = vfs_root();
  uint8_t len = 0;
  console_prompt();
  for (;;) {
    char c = keyboard_getchar();
    if (c == '\n') {
      command[len] = '\0';
      console_putc('\n');
      handle_command(command, &current_dir);
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
