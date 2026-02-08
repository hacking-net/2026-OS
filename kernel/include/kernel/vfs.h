#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include "kernel/types.h"

void vfs_init(void);
void vfs_sanitize(void);
int vfs_write(const char *name, const char *data);
const char *vfs_read(const char *name);
int vfs_remove(const char *name);
int vfs_size(const char *name);
uint8_t vfs_count(void);
const char *vfs_name_at(uint8_t index);
uint8_t vfs_capacity(void);

int vfs_root(void);
int vfs_is_dir(int index);
const char *vfs_name(int index);
int vfs_parent(int index);
int vfs_node_size(int index);
int vfs_resolve(const char *path, int start_dir);
int vfs_resolve_parent(const char *path, int start_dir, char *out_name, uint16_t out_size);
int vfs_mkdir_at(int parent, const char *name);
int vfs_rmdir_at(int parent, const char *name);
int vfs_write_at(int parent, const char *name, const char *data);
const char *vfs_read_at(int parent, const char *name);
int vfs_remove_at(int parent, const char *name);
uint8_t vfs_list_count(int parent);
int vfs_list_at(int parent, uint8_t index);

#endif
