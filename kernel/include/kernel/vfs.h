#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include "kernel/types.h"

void vfs_init(void);
int vfs_write(const char *name, const char *data);
const char *vfs_read(const char *name);
int vfs_remove(const char *name);
int vfs_size(const char *name);
uint8_t vfs_count(void);
const char *vfs_name_at(uint8_t index);
uint8_t vfs_capacity(void);

#endif
