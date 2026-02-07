OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
  . = 1M;

  .multiboot : {
    KEEP(*(.multiboot))
  }

  .text : {
    *(.text .text.*)
  }

  .rodata : {
    *(.rodata .rodata.*)
  }

  .data : {
    *(.data .data.*)
  }

  .bss : {
    *(COMMON)
    *(.bss .bss.*)
  }
}
