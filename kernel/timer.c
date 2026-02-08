#include "kernel/timer.h"
#include "kernel/console.h"
#include "kernel/interrupts.h"
#include "kernel/io.h"
#include "kernel/scheduler.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_BASE_FREQUENCY 1193182

static volatile uint64_t ticks = 0;

void timer_init(uint32_t frequency) {
  if (frequency == 0) {
    frequency = 100;
  }
  uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
  outb(PIT_COMMAND, 0x36);
  outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
  outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
  ticks = 0;
}

uint64_t timer_ticks(void) {
  return ticks;
}

void irq0_handler(void) {
  ticks++;
  scheduler_tick();
  if ((ticks % 100) == 0) {
    console_write_line("tick");
  }
  pic_send_eoi(0);
}
