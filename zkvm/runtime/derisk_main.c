/* Minimal HTIF de-risk program: prints a line and exits 0. Not part of the
 * real guest — used once to validate the bare-metal harness on spike. */
#include "htif.h"

void zkvm_exit(int code) { htif_exit(code); }

int zkvm_start(void)
{
    htif_puts("hello from zkvm guest: HTIF console + exit OK\n");
    return 0;
}
