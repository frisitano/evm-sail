/* ===========================================================================
 * crt0 / reset entry for the eth-act zkVM RISC-V standard target.
 *
 * Machine-mode, no OS, no syscalls.  Responsibilities:
 *   - establish the stack pointer at the top of the STACK region,
 *   - set up the global pointer (gp) for relaxed gp-relative addressing,
 *   - zero the .bss,
 *   - hand control to the C runtime entry zkvm_start(),
 *   - on return, terminate via the standardized termination path.
 * =========================================================================== */

    .section .text.init, "ax", @progbits
    .globl _start
    .type  _start, @function
_start:
    /* Disable linker relaxation while we load gp (it must not be relaxed). */
.option push
.option norelax
    la      gp, __global_pointer$
.option pop

    /* Install the machine-mode trap vector (direct mode; base must be 4-byte
     * aligned).  Any synchronous exception — guard-region access fault, null
     * dereference, instruction-address-misaligned — vectors here and is mapped
     * to standardized ABNORMAL termination instead of an endless trap loop. */
    la      t0, _trap_vector
    csrw    mtvec, t0

    /* Stack pointer -> top of the dedicated STACK region (grows downward). */
    la      sp, __stack_top

    /* Zero .bss: [__bss_start, __bss_end). */
    la      t0, __bss_start
    la      t1, __bss_end
1:
    bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:

    /* Frame anchor for cleanliness, then enter the C runtime. */
    li      fp, 0
    li      ra, 0
    call    zkvm_start

    /* zkvm_start returns an exit code in a0; report it and halt. */
    call    zkvm_exit

    /* zkvm_exit never returns; spin defensively. */
3:  wfi
    j       3b
    .size _start, . - _start

/* ---------------------------------------------------------------------------
 * Machine-mode trap vector.  Reached on any synchronous exception (the guard
 * regions and misaligned-instruction faults among them).  Per the execution-
 * termination-semantics and memory-safety-guard-regions standards, this is an
 * abnormal termination: report the cause to the host and halt.  Control never
 * returns, so we do not preserve the interrupted context.
 * ------------------------------------------------------------------------- */
    .align 4
    .globl _trap_vector
    .type  _trap_vector, @function
_trap_vector:
    csrr    a0, mcause
    csrr    a1, mepc
    csrr    a2, mtval
    la      sp, __stack_top    /* fault may have come from a bad sp; reset it */
    call    zkvm_trap          /* noreturn -> htif_exit(abnormal) */
4:  wfi
    j       4b
    .size _trap_vector, . - _trap_vector
