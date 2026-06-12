/* ===========================================================================
 * Vendor linker script for the eth-act zkVM RISC-V standard target.
 *
 * Target: riscv64im_zicclsm-unknown-none-elf (RV64IM + Zicclsm, LP64 soft-float,
 *         static ELF, little-endian, flat memory, machine mode, NO syscalls).
 *
 * This script implements the mandatory guard regions from the
 * "Memory Safety Guard Regions" standard:
 *
 *   - Null-pointer trap region [0x00000000, 0x00000FFF]:  the first 4 KiB of
 *     the address space is left unmapped.  On the validation simulator (spike)
 *     guest memory begins at 0x80000000, so the entire low range below it —
 *     including the first 4 KiB — is unmapped and any access faults.
 *
 *   - Stack guard region:  a >= 4 KiB unmapped gap sits IMMEDIATELY BELOW the
 *     bottom of the stack.  The stack lives in its own memory region (STACK);
 *     the region directly beneath it (the GAP between MAIN and STACK) is not
 *     handed to the simulator, so a stack overflow that decrements past the
 *     stack bottom lands in unmapped memory and faults (abnormal termination).
 *
 * The simulator must be given memory ranges that match this layout (the GAP is
 * deliberately omitted so it traps):
 *
 *   spike -m0x80000000:0x10000000,0x90010000:0x00100000 ...
 *          \_ MAIN: code+rodata+data+bss+heap    \_ STACK region
 *
 * Layout (addresses):
 *   MAIN  region : 0x80000000 .. 0x90000000   (256 MiB)  text/rodata/data/bss/heap
 *   GUARD gap    : 0x90000000 .. 0x90010000   ( 64 KiB)  UNMAPPED stack guard
 *   STACK region : 0x90010000 .. 0x90110000   (  1 MiB)  stack (grows down)
 * =========================================================================== */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
  MAIN  (rwxa) : ORIGIN = 0x80000000, LENGTH = 0x10000000  /* 256 MiB */
  STACK (rw)   : ORIGIN = 0x90010000, LENGTH = 0x00100000  /*   1 MiB */
}

/* Size of the unmapped guard gap immediately below the stack bottom (>= 4 KiB).
 * It is the distance between the end of MAIN and the start of STACK. */
__stack_guard_size = ORIGIN(STACK) - (ORIGIN(MAIN) + LENGTH(MAIN));

/* Stack: bottom at the start of the STACK region, top (initial sp) at its end. */
__stack_bottom = ORIGIN(STACK);
__stack_top    = ORIGIN(STACK) + LENGTH(STACK);

SECTIONS
{
  . = ORIGIN(MAIN);

  /* Reset/entry must be first so spike's default reset vector (-> 0x80000000)
   * lands on _start. */
  .text : ALIGN(4)
  {
    KEEP(*(.text.init))      /* _start */
    *(.text .text.*)
  } > MAIN

  .rodata : ALIGN(8)
  {
    *(.srodata .srodata.*)
    *(.rodata .rodata.*)
  } > MAIN

  .data : ALIGN(8)
  {
    *(.data .data.*)
    /* Global pointer anchors the small-data area for gp-relative addressing. */
    . = ALIGN(8);
    PROVIDE(__global_pointer$ = . + 0x800);
    *(.sdata .sdata.*)
  } > MAIN

  /* HTIF console mailbox — fesvr/spike locate these by symbol name. Must live
   * in mapped, writable memory and be 8-byte aligned. */
  .htif : ALIGN(8)
  {
    PROVIDE(tohost = .);   . += 8;
    PROVIDE(fromhost = .); . += 8;
  } > MAIN

  .bss (NOLOAD) : ALIGN(8)
  {
    __bss_start = .;
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    __bss_end = .;
  } > MAIN

  /* Heap: everything left in MAIN after bss, up to the end of the region. */
  . = ALIGN(16);
  __heap_start = .;
  __heap_end   = ORIGIN(MAIN) + LENGTH(MAIN);

  /DISCARD/ :
  {
    *(.comment)
    *(.note .note.*)
    *(.eh_frame .eh_frame.*)
    *(.riscv.attributes)
  }
}
