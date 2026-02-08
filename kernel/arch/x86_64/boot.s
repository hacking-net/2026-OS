.set MB2_MAGIC, 0xe85250d6
.set MB2_ARCH, 0
.set MB2_HEADER_LENGTH, (mb2_header_end - mb2_header)
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_HEADER_LENGTH)

.set CR0_PE, 0x1
.set CR0_PG, 0x80000000
.set CR4_PAE, 0x20
.set EFER_MSR, 0xC0000080
.set EFER_LME, 0x100

.section .multiboot
.align 8
mb2_header:
  .long MB2_MAGIC
  .long MB2_ARCH
  .long MB2_HEADER_LENGTH
  .long MB2_CHECKSUM
  .short 0
  .short 0
  .long 8
mb2_header_end:

.section .text
.global _start
.code32
_start:
  cli
  mov $stack_top, %esp

  call setup_paging
  lgdt gdt64_ptr

  mov $EFER_MSR, %ecx
  rdmsr
  or $EFER_LME, %eax
  wrmsr

  mov %cr4, %eax
  or $CR4_PAE, %eax
  mov %eax, %cr4

  mov $pml4, %eax
  mov %eax, %cr3

  mov %cr0, %eax
  or $(CR0_PG | CR0_PE), %eax
  mov %eax, %cr0

  ljmp $GDT64_CODE, $long_mode_entry

setup_paging:
  mov $pd, %edi
  xor %ebx, %ebx
  mov $512, %ecx
1:
  mov %ebx, %eax
  or $0x83, %eax
  mov %eax, (%edi)
  movl $0, 4(%edi)
  add $0x200000, %ebx
  add $8, %edi
  loop 1b

  mov $pd, %eax
  or $0x3, %eax
  movl %eax, pdpt
  movl $0, pdpt+4

  mov $pdpt, %eax
  or $0x3, %eax
  movl %eax, pml4
  movl $0, pml4+4

  ret

.code64
long_mode_entry:
  mov $GDT64_DATA, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss

  mov $stack_top, %rsp
  call kernel_main

.hang:
  hlt
  jmp .hang

.section .rodata
.align 8
gdt64:
  .quad 0x0000000000000000
  .quad 0x00209A0000000000
  .quad 0x0000920000000000

gdt64_ptr:
  .word gdt64_ptr_end - gdt64 - 1
  .quad gdt64

gdt64_ptr_end:

.set GDT64_CODE, 0x08
.set GDT64_DATA, 0x10

.section .bss
.align 4096
pml4:
  .skip 4096
pdpt:
  .skip 4096
pd:
  .skip 4096

.align 16
stack:
  .skip 16384
stack_top:
