#include "kernel/init.h"
#include "kernel/ipc.h"
#include "kernel/mm.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/vfs.h"

void kernel_init(void) {
  mm_init();
  scheduler_init();
  process_init();
  ipc_init();
  vfs_init();
}
