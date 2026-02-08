OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
  /* Keep the Multiboot2 header within the first 32 KiB of the file. */
  . = 0;

  .multiboot : {
    KEEP(*(.multiboot))
  }

  ASSERT(ADDR(.multiboot) < 32K, "Multiboot2 header must be in first 32 KiB");

  . = 1M;

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
