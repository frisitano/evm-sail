/* ===========================================================================
 * HTIF implementation (spike validation harness only).
 *
 * The `tohost` / `fromhost` pair is the fesvr mailbox.  A 64-bit `tohost`
 * value is encoded as:
 *
 *     bits [63:56] = device
 *     bits [55:48] = command
 *     bits [47:0]  = payload
 *
 * Console output uses device 1 ("bcd"), command 1 (write a character in the low
 * byte).  Exit uses device 0, command 0 with an odd payload: the host reads the
 * exit code as payload >> 1.  After each request the host acknowledges by
 * writing `fromhost`, which the guest must clear before the next request.
 * =========================================================================== */

#include "htif.h"

/* Defined by the linker script, in mapped writable memory, 8-byte aligned. */
extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

static void htif_send(uint64_t dev, uint64_t cmd, uint64_t payload)
{
    /* Wait for any previous request to be drained by the host (fesvr clears
     * `tohost` once it has handled the command). */
    while (tohost != 0) {
        /* spin */
    }
    /* If the host posted an unrelated fromhost (e.g. an input notification),
     * acknowledge it so it doesn't wedge the next exchange. */
    if (fromhost != 0) {
        fromhost = 0;
    }
    tohost = (dev << 56) | (cmd << 48) | (payload & 0xffffffffffffULL);
}

void htif_putchar(char c)
{
    /* Console writes (device 1, command 1) are fire-and-forget: fesvr prints
     * the low byte and clears `tohost`. There is no fromhost acknowledgement
     * for a write, so we must not wait on it. */
    htif_send(1, 1, (uint64_t)(uint8_t)c);
    while (tohost != 0) {
        /* ensure this character is drained before the next one */
    }
}

void htif_puts(const char *s)
{
    if (!s) {
        return;
    }
    while (*s) {
        htif_putchar(*s++);
    }
}

void htif_exit(int code)
{
    /* Exit request: device 0, command 0, payload = (code << 1) | 1.
     * No acknowledgement handshake — the host halts the machine. */
    tohost = ((uint64_t)(uint32_t)code << 1) | 1ULL;
    for (;;) {
        /* unreachable once the host halts */
    }
}
