#include "kernel/interrupts.h"
#include "kernel/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define IDT_TYPE_INTERRUPT 0x8E

struct idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];

extern void irq0_stub(void);
extern void isr_stub(void);

static void idt_set_gate(uint8_t vector, void (*handler)(void)) {
  uint64_t addr = (uint64_t)handler;
  idt[vector].offset_low = (uint16_t)(addr & 0xFFFF);
  idt[vector].selector = 0x08;
  idt[vector].ist = 0;
  idt[vector].type_attr = IDT_TYPE_INTERRUPT;
  idt[vector].offset_mid = (uint16_t)((addr >> 16) & 0xFFFF);
  idt[vector].offset_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
  idt[vector].zero = 0;
}

static void idt_load(const struct idt_ptr *desc) {
  __asm__ volatile("lidt (%0)" : : "r"(desc));
}

static void pic_remap(void) {
  uint8_t a1 = inb(PIC1_DATA);
  uint8_t a2 = inb(PIC2_DATA);

  outb(PIC1_COMMAND, 0x11);
  outb(PIC2_COMMAND, 0x11);
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);
  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);
  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);

  outb(PIC1_DATA, a1);
  outb(PIC2_DATA, a2);
}

void pic_send_eoi(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC2_COMMAND, PIC_EOI);
  }
  outb(PIC1_COMMAND, PIC_EOI);
}

void interrupts_init(void) {
  for (uint16_t i = 0; i < 256; ++i) {
    idt[i].offset_low = 0;
    idt[i].selector = 0;
    idt[i].ist = 0;
    idt[i].type_attr = 0;
    idt[i].offset_mid = 0;
    idt[i].offset_high = 0;
    idt[i].zero = 0;
  }
  for (uint8_t vec = 0; vec < 32; ++vec) {
    idt_set_gate(vec, isr_stub);
  }
  idt_set_gate(0x20, irq0_stub);

  struct idt_ptr desc;
  desc.limit = (uint16_t)(sizeof(idt) - 1);
  desc.base = (uint64_t)idt;
  idt_load(&desc);

  pic_remap();
  outb(PIC1_DATA, 0xFE);
  outb(PIC2_DATA, 0xFF);
}

void interrupts_enable(void) {
  __asm__ volatile("sti");
}

void interrupts_disable(void) {
  __asm__ volatile("cli");
}
