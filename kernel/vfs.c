#include "kernel/vfs.h"

#define VFS_MAX_FILES 8
#define VFS_NAME_MAX 16
#define VFS_DATA_MAX 128

typedef struct {
  char name[VFS_NAME_MAX];
  char data[VFS_DATA_MAX];
  uint16_t size;
  uint8_t used;
} vfs_file_t;

static vfs_file_t vfs_files[VFS_MAX_FILES];

static uint16_t vfs_strlen(const char *s) {
  uint16_t len = 0;
  while (s && s[len]) {
    len++;
  }
  return len;
}

static void vfs_strcpy(char *dest, const char *src, uint16_t max) {
  uint16_t i = 0;
  for (; i + 1 < max && src[i]; ++i) {
    dest[i] = src[i];
  }
  dest[i] = '\0';
}

static int vfs_streq(const char *a, const char *b) {
  uint16_t i = 0;
  while (a[i] && b[i]) {
    if (a[i] != b[i]) {
      return 0;
    }
    i++;
  }
  return a[i] == b[i];
}

static int vfs_find_index(const char *name) {
  for (uint8_t i = 0; i < VFS_MAX_FILES; ++i) {
    if (vfs_files[i].used && vfs_streq(vfs_files[i].name, name)) {
      return (int)i;
    }
  }
  return -1;
}

void vfs_init(void) {
  for (uint8_t i = 0; i < VFS_MAX_FILES; ++i) {
    vfs_files[i].used = 0;
    vfs_files[i].size = 0;
    vfs_files[i].name[0] = '\0';
    vfs_files[i].data[0] = '\0';
  }
  vfs_write("readme.txt", "Witaj w 2026-OS!\n");
}

int vfs_write(const char *name, const char *data) {
  if (!name || !name[0]) {
    return -1;
  }
  if (vfs_strlen(name) >= VFS_NAME_MAX) {
    return -2;
  }
  uint16_t data_len = vfs_strlen(data);
  if (data_len >= VFS_DATA_MAX) {
    return -3;
  }
  int index = vfs_find_index(name);
  if (index < 0) {
    for (uint8_t i = 0; i < VFS_MAX_FILES; ++i) {
      if (!vfs_files[i].used) {
        index = (int)i;
        vfs_files[i].used = 1;
        vfs_strcpy(vfs_files[i].name, name, VFS_NAME_MAX);
        break;
      }
    }
  }
  if (index < 0) {
    return -4;
  }
  vfs_strcpy(vfs_files[index].data, data, VFS_DATA_MAX);
  vfs_files[index].size = data_len;
  return 0;
}

const char *vfs_read(const char *name) {
  int index = vfs_find_index(name);
  if (index < 0) {
    return 0;
  }
  return vfs_files[index].data;
}

int vfs_remove(const char *name) {
  int index = vfs_find_index(name);
  if (index < 0) {
    return -1;
  }
  vfs_files[index].used = 0;
  vfs_files[index].size = 0;
  vfs_files[index].name[0] = '\0';
  vfs_files[index].data[0] = '\0';
  return 0;
}

int vfs_size(const char *name) {
  int index = vfs_find_index(name);
  if (index < 0) {
    return -1;
  }
  return vfs_files[index].size;
}

uint8_t vfs_count(void) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < VFS_MAX_FILES; ++i) {
    if (vfs_files[i].used) {
      count++;
    }
  }
  return count;
}

const char *vfs_name_at(uint8_t index) {
  uint8_t seen = 0;
  for (uint8_t i = 0; i < VFS_MAX_FILES; ++i) {
    if (vfs_files[i].used) {
      if (seen == index) {
        return vfs_files[i].name;
      }
      seen++;
    }
  }
  return 0;
}

uint8_t vfs_capacity(void) {
  return VFS_MAX_FILES;
}
