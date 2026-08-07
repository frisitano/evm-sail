/* ZisK platform adapter for the shared freestanding evm-sail runtime. */

#include <stddef.h>
#include <stdint.h>

#include "htif.h"

#ifndef EVMSAIL_ZISK_HEAP_SIZE
#define EVMSAIL_ZISK_HEAP_SIZE (UINT64_C(256) * UINT64_C(1024) * UINT64_C(1024))
#endif

extern void *sys_alloc_aligned(size_t bytes, size_t align);
extern void sys_write(uint32_t fd, const uint8_t *data, size_t size);

void heap_region(char **start, char **end)
{
    static char *heap_start;

    if (heap_start == NULL) {
        heap_start = sys_alloc_aligned((size_t)EVMSAIL_ZISK_HEAP_SIZE, 16);
    }
    *start = heap_start;
    *end = heap_start + (size_t)EVMSAIL_ZISK_HEAP_SIZE;
}

void htif_putchar(char c)
{
    sys_write(2, (const uint8_t *)&c, 1);
}

void htif_puts(const char *s)
{
    const char *end = s;
    while (*end != '\0') {
        end++;
    }
    sys_write(2, (const uint8_t *)s, (size_t)(end - s));
}

#ifdef EVMSAIL_DEBUG
extern _Bool validation_failure_present;
extern uint64_t validation_failure_scope;
extern int validation_failure_reason;

static char hex_digit(unsigned value)
{
    return (char)(value < 10 ? '0' + value : 'a' + value - 10);
}

void zisk_report_debug(void)
{
    char line[] = "validation_failure=0 scope=00 reason=00\n";
    unsigned scope = (unsigned)validation_failure_scope & 0xffu;
    unsigned reason = (unsigned)validation_failure_reason & 0xffu;

    line[19] = validation_failure_present ? '1' : '0';
    line[27] = hex_digit(scope >> 4);
    line[28] = hex_digit(scope & 0xfu);
    line[37] = hex_digit(reason >> 4);
    line[38] = hex_digit(reason & 0xfu);
    sys_write(2, (const uint8_t *)line, sizeof line - 1);
    /* ziskemu discards guest stderr and does not propagate the guest exit
     * status, so append the verdict to the public output where the padding
     * of the fixed 256-byte ZisK output region makes it directly visible. */
    {
        extern void write_output(const uint8_t *output, size_t size);
        extern _Bool validation_debug_gas_present;
        extern uint64_t validation_debug_header_gas_actual;
        extern uint64_t validation_debug_header_gas_expected;
        extern uint64_t validation_debug_execution_gas;
        extern uint64_t validation_debug_state_gas;
        uint64_t gas[4];
        uint8_t tag[7 + sizeof gas];
        tag[0] = 0xdb;
        tag[1] = 0x9a;
        tag[2] = validation_failure_present ? 1u : 0u;
        tag[3] = (uint8_t)scope;
        tag[4] = (uint8_t)reason;
        tag[5] = validation_debug_gas_present ? 1u : 0u;
        tag[6] = 0xdb;
        gas[0] = validation_debug_header_gas_actual;
        gas[1] = validation_debug_header_gas_expected;
        gas[2] = validation_debug_execution_gas;
        gas[3] = validation_debug_state_gas;
        for (unsigned i = 0; i < sizeof gas; i++) {
            tag[7 + i] = ((const uint8_t *)gas)[i];
        }
        write_output(tag, sizeof tag);
    }
}
#endif

void htif_exit(int code)
{
    __asm__ volatile(
        "mv a0, %0\n"
        "li a7, 93\n"
        "ecall\n"
        :
        : "r"((long)code)
        : "a0", "a7", "memory");
    for (;;) {
        __asm__ volatile("" ::: "memory");
    }
}
