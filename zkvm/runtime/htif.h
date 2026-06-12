/* ===========================================================================
 * Minimal HTIF (host-target interface) for the spike validation harness.
 *
 * HTIF is the bare-metal console / exit channel that spike (fesvr) drives via
 * the `tohost` / `fromhost` symbols.  It is used ONLY by the validation
 * runtime to (a) emit a human-readable debug line and (b) implement the
 * standardized termination semantics (halt + report exit code to the host).
 *
 * This is the harness's stand-in for a real zkVM host.  No part of the guest
 * computation depends on it.
 * =========================================================================== */
#ifndef ZKVM_HTIF_H
#define ZKVM_HTIF_H

#include <stdint.h>
#include <stddef.h>

/* Emit one byte to the host console (HTIF device 1, command 1). */
void htif_putchar(char c);

/* Emit a NUL-terminated string to the host console. */
void htif_puts(const char *s);

/* Standardized termination: halt execution and report `code` to the host.
 * code == 0  => successful termination.
 * code != 0  => abnormal termination (the host/verifier observes the code).
 * Never returns. */
void htif_exit(int code) __attribute__((noreturn));

#endif /* ZKVM_HTIF_H */
