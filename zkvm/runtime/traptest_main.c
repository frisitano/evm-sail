/* Guard-region enforcement test (validation harness only).
 *
 * Reads from the null-pointer trap region (address 0). On the standard target
 * that region is unmapped, so the access raises a fault that the platform trap
 * vector maps to ABNORMAL termination (non-zero exit). Demonstrates that the
 * guard regions in link.ld actually trap rather than silently corrupt memory.
 *
 * Build mode is selected by ZKVM_TRAP_KIND:
 *   0 (default) -> null-pointer read at 0x0
 *   1           -> read just below the stack bottom (into the stack guard gap)
 */
#include <stdint.h>
#include "htif.h"

void zkvm_exit(int code) { htif_exit(code); }

extern char __stack_bottom[];

int zkvm_start(void)
{
#ifndef ZKVM_TRAP_KIND
#define ZKVM_TRAP_KIND 0
#endif
#if ZKVM_TRAP_KIND == 1
    htif_puts("[traptest] reading the stack guard region (below stack bottom)\n");
    volatile char *p = (volatile char *)(__stack_bottom - 64);
#else
    htif_puts("[traptest] reading the null-pointer trap region (0x0)\n");
    volatile char *p = (volatile char *)0;
#endif
    char v = *p;                 /* expected to fault -> abnormal termination */
    htif_puts("[traptest] ERROR: guard access did NOT trap (read value)\n");
    return (int)v | 1;           /* if we get here, the guard failed: non-zero */
}
