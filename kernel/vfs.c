#include "kernel/vfs.h"

#define VFS_MAX_NODES 32
#define VFS_NAME_MAX 16
#define VFS_DATA_MAX 128

typedef enum {
  VFS_NODE_DIR = 1,
  VFS_NODE_FILE = 2
} vfs_node_type_t;

typedef struct {
  char name[VFS_NAME_MAX];
  char data[VFS_DATA_MAX];
  uint16_t size;
  int16_t parent;
  uint8_t used;
  uint8_t type;
} vfs_node_t;

static vfs_node_t vfs_nodes[VFS_MAX_NODES];

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

static int vfs_find_free(void) {
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (!vfs_nodes[i].used) {
      return (int)i;
    }
  }
  return -1;
}

static int vfs_find_child(int parent, const char *name) {
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (vfs_nodes[i].used && vfs_nodes[i].parent == parent &&
        vfs_streq(vfs_nodes[i].name, name)) {
      return (int)i;
    }
  }
  return -1;
}

static uint8_t vfs_split(const char *path, char *part, uint16_t part_size, uint16_t *offset) {
  uint16_t i = *offset;
  while (path[i] == '/') {
    i++;
  }
  if (!path[i]) {
    return 0;
  }
  uint16_t out = 0;
  while (path[i] && path[i] != '/' && out + 1 < part_size) {
    part[out++] = path[i++];
  }
  part[out] = '\0';
  *offset = i;
  return 1;
}

static void vfs_clear_node(int index) {
  vfs_nodes[index].used = 0;
  vfs_nodes[index].size = 0;
  vfs_nodes[index].parent = -1;
  vfs_nodes[index].type = 0;
  vfs_nodes[index].name[0] = '\0';
  vfs_nodes[index].data[0] = '\0';
}

void vfs_init(void) {
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    vfs_clear_node(i);
  }
  vfs_nodes[0].used = 1;
  vfs_nodes[0].type = VFS_NODE_DIR;
  vfs_nodes[0].parent = -1;
  vfs_strcpy(vfs_nodes[0].name, "/", VFS_NAME_MAX);
  vfs_write_at(0, "readme.txt", "Witaj w 2026-OS!\n");
}

int vfs_root(void) {
  return 0;
}

int vfs_is_dir(int index) {
  if (index < 0 || index >= VFS_MAX_NODES || !vfs_nodes[index].used) {
    return 0;
  }
  return vfs_nodes[index].type == VFS_NODE_DIR;
}

const char *vfs_name(int index) {
  if (index < 0 || index >= VFS_MAX_NODES || !vfs_nodes[index].used) {
    return 0;
  }
  return vfs_nodes[index].name;
}

int vfs_parent(int index) {
  if (index < 0 || index >= VFS_MAX_NODES || !vfs_nodes[index].used) {
    return -1;
  }
  return vfs_nodes[index].parent;
}

int vfs_node_size(int index) {
  if (index < 0 || index >= VFS_MAX_NODES || !vfs_nodes[index].used) {
    return -1;
  }
  if (vfs_nodes[index].type != VFS_NODE_FILE) {
    return -1;
  }
  return vfs_nodes[index].size;
}

int vfs_resolve(const char *path, int start_dir) {
  if (!path || !path[0]) {
    return start_dir;
  }
  int current = (path[0] == '/') ? vfs_root() : start_dir;
  uint16_t offset = 0;
  char part[VFS_NAME_MAX];
  while (vfs_split(path, part, VFS_NAME_MAX, &offset)) {
    if (vfs_streq(part, ".")) {
      continue;
    }
    if (vfs_streq(part, "..")) {
      int parent = vfs_parent(current);
      if (parent >= 0) {
        current = parent;
      }
      continue;
    }
    int next = vfs_find_child(current, part);
    if (next < 0) {
      return -1;
    }
    current = next;
  }
  return current;
}

int vfs_resolve_parent(const char *path, int start_dir, char *out_name, uint16_t out_size) {
  if (!path || !path[0]) {
    return -1;
  }
  int current = (path[0] == '/') ? vfs_root() : start_dir;
  uint16_t offset = 0;
  char part[VFS_NAME_MAX];
  char last[VFS_NAME_MAX];
  last[0] = '\0';
  while (vfs_split(path, part, VFS_NAME_MAX, &offset)) {
    if (vfs_streq(part, ".")) {
      continue;
    }
    if (vfs_streq(part, "..")) {
      int parent = vfs_parent(current);
      if (parent >= 0) {
        current = parent;
      }
      continue;
    }
    vfs_strcpy(last, part, VFS_NAME_MAX);
    if (path[offset] == '/') {
      int next = vfs_find_child(current, part);
      if (next < 0 || !vfs_is_dir(next)) {
        return -1;
      }
      current = next;
    }
  }
  if (!last[0]) {
    return -1;
  }
  vfs_strcpy(out_name, last, out_size);
  return current;
}

int vfs_mkdir_at(int parent, const char *name) {
  if (!name || !name[0]) {
    return -1;
  }
  if (vfs_strlen(name) >= VFS_NAME_MAX) {
    return -2;
  }
  if (parent < 0 || !vfs_is_dir(parent)) {
    return -3;
  }
  if (vfs_find_child(parent, name) >= 0) {
    return -4;
  }
  int slot = vfs_find_free();
  if (slot < 0) {
    return -5;
  }
  vfs_nodes[slot].used = 1;
  vfs_nodes[slot].type = VFS_NODE_DIR;
  vfs_nodes[slot].parent = parent;
  vfs_strcpy(vfs_nodes[slot].name, name, VFS_NAME_MAX);
  vfs_nodes[slot].size = 0;
  vfs_nodes[slot].data[0] = '\0';
  return 0;
}

int vfs_rmdir_at(int parent, const char *name) {
  int index = vfs_find_child(parent, name);
  if (index < 0) {
    return -1;
  }
  if (!vfs_is_dir(index)) {
    return -2;
  }
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (vfs_nodes[i].used && vfs_nodes[i].parent == index) {
      return -3;
    }
  }
  vfs_clear_node(index);
  return 0;
}

int vfs_write_at(int parent, const char *name, const char *data) {
  if (!name || !name[0]) {
    return -1;
  }
  if (vfs_strlen(name) >= VFS_NAME_MAX) {
    return -2;
  }
  if (parent < 0 || !vfs_is_dir(parent)) {
    return -3;
  }
  uint16_t data_len = vfs_strlen(data);
  if (data_len >= VFS_DATA_MAX) {
    return -4;
  }
  int index = vfs_find_child(parent, name);
  if (index < 0) {
    int slot = vfs_find_free();
    if (slot < 0) {
      return -5;
    }
    index = slot;
    vfs_nodes[index].used = 1;
    vfs_nodes[index].type = VFS_NODE_FILE;
    vfs_nodes[index].parent = parent;
    vfs_strcpy(vfs_nodes[index].name, name, VFS_NAME_MAX);
  } else if (vfs_is_dir(index)) {
    return -6;
  }
  vfs_strcpy(vfs_nodes[index].data, data, VFS_DATA_MAX);
  vfs_nodes[index].size = data_len;
  return 0;
}

const char *vfs_read_at(int parent, const char *name) {
  int index = vfs_find_child(parent, name);
  if (index < 0 || vfs_is_dir(index)) {
    return 0;
  }
  return vfs_nodes[index].data;
}

int vfs_remove_at(int parent, const char *name) {
  int index = vfs_find_child(parent, name);
  if (index < 0 || vfs_is_dir(index)) {
    return -1;
  }
  vfs_clear_node(index);
  return 0;
}

uint8_t vfs_list_count(int parent) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (vfs_nodes[i].used && vfs_nodes[i].parent == parent) {
      count++;
    }
  }
  return count;
}

int vfs_list_at(int parent, uint8_t index) {
  uint8_t seen = 0;
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (vfs_nodes[i].used && vfs_nodes[i].parent == parent) {
      if (seen == index) {
        return (int)i;
      }
      seen++;
    }
  }
  return -1;
}

int vfs_write(const char *name, const char *data) {
  return vfs_write_at(vfs_root(), name, data);
}

const char *vfs_read(const char *name) {
  return vfs_read_at(vfs_root(), name);
}

int vfs_remove(const char *name) {
  return vfs_remove_at(vfs_root(), name);
}

int vfs_size(const char *name) {
  int index = vfs_find_child(vfs_root(), name);
  if (index < 0 || vfs_is_dir(index)) {
    return -1;
  }
  return vfs_nodes[index].size;
}

uint8_t vfs_count(void) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < VFS_MAX_NODES; ++i) {
    if (vfs_nodes[i].used) {
      count++;
    }
  }
  return count;
}

const char *vfs_name_at(uint8_t index) {
  int node = vfs_list_at(vfs_root(), index);
  if (node < 0) {
    return 0;
  }
  return vfs_nodes[node].name;
}

uint8_t vfs_capacity(void) {
  return VFS_MAX_NODES;
}
