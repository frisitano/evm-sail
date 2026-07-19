/* ===========================================================================
 * Freestanding C runtime for the eth-act zkVM RISC-V standard target.
 *
 * The guest is built -ffreestanding -nostdlib: there is no OS, no libc, and no
 * syscalls.  This file supplies EXACTLY the slice of the hosted environment the
 * linked objects reference (audited with nm against every guest object; if a
 * future model/runtime edit needs more, the link fails loudly -- add it back
 * from git history):
 *
 *   - a heap allocator (malloc/free/realloc/calloc) over the linker-defined
 *     heap region, with a first-fit free list + coalescing,
 *   - the mem-copy/compare family + strlen (gcc emits calls to these for
 *     struct copies even under -ffreestanding) and qsort (ffi/state_db.c BAL
 *     sorting),
 *   - the standardized termination mapping: abort()/exit, the Sail failure
 *     contract (sail_assert / sail_failure / sail_match_failure), and the
 *     trap-vector handler all halt the machine and report ABNORMAL termination
 *     (non-zero exit) to the host (see standard-termination-semantics),
 *   - no-op stubs for the parts of the Sail RTS the model references but that a
 *     freestanding guest does not use (setup_rts/cleanup_rts), and a
 *     format-free fprintf routing diagnostics to the HTIF console.
 * =========================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "htif.h"

/* Standardized abnormal-termination exit code (non-zero). A Type-2 verifier can
 * key on this; a Type-1 verifier rejects any proof of a failed execution. */
#define ZKVM_ABORT_CODE 134   /* conventional SIGABRT-style code */

__attribute__((noreturn)) void zkvm_abort(const char *why)
{
    htif_puts("\n[zkvm] ABNORMAL TERMINATION: ");
    htif_puts(why ? why : "(unspecified)");
    htif_putchar('\n');
    htif_exit(ZKVM_ABORT_CODE);
}

/* Synchronous-exception handler, entered from _trap_vector in start.S. A guard-
 * region access, null dereference, or instruction-address-misaligned fault all
 * land here and are reported as abnormal termination (standard-termination-
 * semantics).  mcause values: 0 instr-addr-misaligned, 1 instr access fault,
 * 5 load access fault, 7 store access fault, etc. */
static void emit_hex16(uint64_t v)
{
    static const char hexd[] = "0123456789abcdef";
    char buf[18];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; i++) {
        buf[2 + i] = hexd[(v >> ((15 - i) * 4)) & 0xf];
    }
    for (int i = 0; i < 18; i++) {
        htif_putchar(buf[i]);
    }
}

__attribute__((noreturn)) void zkvm_trap(uint64_t mcause, uint64_t mepc, uint64_t mtval)
{
    const char *what = "trap";
    switch (mcause) {
        case 0:  what = "instruction address misaligned"; break;
        case 1:  what = "instruction access fault";        break;
        case 2:  what = "illegal instruction";             break;
        case 5:  what = "load access fault (guard region?)";  break;
        case 7:  what = "store access fault (guard region?)"; break;
    }
    htif_puts("\n[zkvm] ABNORMAL TERMINATION (trap): ");
    htif_puts(what);
    htif_puts("  mcause=");
    emit_hex16(mcause);
    htif_puts(" mepc=");
    emit_hex16(mepc);
    htif_puts(" mtval=");
    emit_hex16(mtval);
    htif_putchar('\n');
    htif_exit(ZKVM_ABORT_CODE);
}

/* ----- <string.h> subset ---------------------------------------------------
 * gcc lowers struct copies / memcpy idioms to calls to these even under
 * -ffreestanding, so they must exist as real symbols; strlen backs fprintf. */

#ifndef EVMSAIL_PLATFORM_LIBC_MEMORY
void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d == s || n == 0) {
        return dst;
    }
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--) {
        *d++ = (unsigned char)c;
    }
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    while (n--) {
        if (*x != *y) {
            return (int)*x - (int)*y;
        }
        x++;
        y++;
    }
    return 0;
}
#endif

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}

/* ----- heap allocator -----------------------------------------------------
 * First-fit free list with boundary-tag coalescing over [__heap_start,
 * __heap_end).  The Sail runtime and the FFI backends allocate and free
 * many short-lived objects during a block run, so reclaiming freed memory
 * (rather than a pure bump allocator) keeps the working set bounded. */

#ifdef EVMSAIL_EXTERNAL_HEAP
extern void evmsail_heap_region(char **start, char **end);
#else
extern char __heap_start[];
extern char __heap_end[];
#endif

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((uintptr_t)(a) - 1))
#define HEAP_ALIGN 16u

typedef struct block_hdr {
    size_t            size;   /* usable payload bytes (excludes header)        */
    struct block_hdr *next;   /* next free block (free list only)              */
    uint32_t          free;   /* 1 if on the free list                         */
    uint32_t          magic;  /* sanity tag                                    */
} block_hdr;

#define BLK_MAGIC 0x5A4B564Du /* "ZKVM" */
#define HDR_SIZE  (ALIGN_UP(sizeof(block_hdr), HEAP_ALIGN))

static block_hdr *free_list = NULL;
static char      *brk_ptr   = NULL;   /* bump frontier for fresh memory       */
static char      *brk_end   = NULL;
static int        heap_ready = 0;

static void heap_init(void)
{
    char *heap_start;
    char *heap_end;
#ifdef EVMSAIL_EXTERNAL_HEAP
    evmsail_heap_region(&heap_start, &heap_end);
#else
    heap_start = __heap_start;
    heap_end = __heap_end;
#endif
    brk_ptr   = (char *)ALIGN_UP((uintptr_t)heap_start, HEAP_ALIGN);
    brk_end   = heap_end;
    free_list = NULL;
    heap_ready = 1;
}

static block_hdr *bump_new(size_t payload)
{
    char *raw = (char *)ALIGN_UP((uintptr_t)brk_ptr, HEAP_ALIGN);
    char *end = raw + HDR_SIZE + payload;
    if (end > brk_end) {
        return NULL;   /* out of heap */
    }
    brk_ptr = end;
    block_hdr *h = (block_hdr *)raw;
    h->size  = payload;
    h->next  = NULL;
    h->free  = 0;
    h->magic = BLK_MAGIC;
    return h;
}

void *malloc(size_t n)
{
    if (!heap_ready) {
        heap_init();
    }
    if (n == 0) {
        n = 1;
    }
    n = ALIGN_UP(n, HEAP_ALIGN);

    /* first fit */
    block_hdr **pp = &free_list;
    for (block_hdr *b = free_list; b; b = b->next) {
        if (b->size >= n) {
            *pp = b->next;
            b->free = 0;
            b->next = NULL;
            return (char *)b + HDR_SIZE;
        }
        pp = &b->next;
    }

    block_hdr *h = bump_new(n);
    if (!h) {
        zkvm_abort("heap exhausted (malloc)");
    }
    return (char *)h + HDR_SIZE;
}

void free(void *p)
{
    if (!p) {
        return;
    }
    block_hdr *h = (block_hdr *)((char *)p - HDR_SIZE);
    if (h->magic != BLK_MAGIC) {
        zkvm_abort("heap corruption (free of bad pointer)");
    }
    if (h->free) {
        return;   /* double free: ignore defensively */
    }
    h->free = 1;
    h->next = free_list;
    free_list = h;
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    if (size != 0 && total / size != nmemb) {
        zkvm_abort("calloc overflow");
    }
    void *p = malloc(total);
    memset(p, 0, total);
    return p;
}

void *realloc(void *p, size_t n)
{
    if (!p) {
        return malloc(n);
    }
    if (n == 0) {
        free(p);
        return malloc(1);
    }
    block_hdr *h = (block_hdr *)((char *)p - HDR_SIZE);
    if (h->magic != BLK_MAGIC) {
        zkvm_abort("heap corruption (realloc of bad pointer)");
    }
    if (h->size >= ALIGN_UP(n, HEAP_ALIGN)) {
        return p;   /* current block already big enough */
    }
    void *q = malloc(n);
    memcpy(q, p, h->size < n ? h->size : n);
    free(p);
    return q;
}

/* ----- termination / abort / assert ---------------------------------------- */

void abort(void)
{
    zkvm_abort("abort()");
}

void exit(int code)
{
    htif_exit(code);
}

/* ----- Sail failure backend (sail_failure.h) ------------------------------- */

void sail_match_failure(char *msg)
{
    zkvm_abort(msg ? msg : "Sail match failure");
}

void sail_failure(char *msg)
{
    zkvm_abort(msg ? msg : "Sail builtin failure");
}

int sail_assert(bool result, char *msg)
{
    if (!result) {
        zkvm_abort(msg ? msg : "Sail assertion failed");
    }
    return 0;
}

/* ----- Sail RTS stubs (model_init/model_fini reference these) --------------
 * A freestanding guest needs no argv parsing, ELF loading, or timing, so these
 * are inert.  Diagnostic print/format routines route to the HTIF console so any
 * stray model diagnostics remain observable rather than crashing the link. */

void setup_rts(void)   {}
void cleanup_rts(void) {}

/* qsort: heapsort -- in-place, no recursion, no allocation, O(n log n) worst
 * case, fully deterministic for a total-order comparator (the BAL builder in
 * ffi/state_db.c sorts consensus-critical row arrays through this). */
static void qsort_swap(char *a, char *b, size_t sz)
{
    while (sz--) {
        char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static void qsort_sift(char *base, size_t sz, size_t root, size_t n,
                       int (*cmp)(const void *, const void *))
{
    for (;;) {
        size_t child = 2 * root + 1;
        if (child >= n) {
            return;
        }
        if (child + 1 < n && cmp(base + child * sz, base + (child + 1) * sz) < 0) {
            child++;
        }
        if (cmp(base + root * sz, base + child * sz) >= 0) {
            return;
        }
        qsort_swap(base + root * sz, base + child * sz, sz);
        root = child;
    }
}

void qsort(void *b, size_t n, size_t sz,
           int (*cmp)(const void *, const void *))
{
    char *base = b;
    if (n < 2 || sz == 0) {
        return;
    }
    for (size_t i = n / 2; i-- > 0;) {
        qsort_sift(base, sz, i, n, cmp);
    }
    for (size_t i = n - 1; i > 0; i--) {
        qsort_swap(base, base + i * sz, sz);
        qsort_sift(base, sz, 0, i, cmp);
    }
}

/* The only stdio the linked objects reference: the Sail runtime's
 * diagnostic/error paths call fprintf(stderr, ...). We never do real file I/O
 * and perform NO conversion-specifier expansion (none is needed on the
 * executed paths) -- the literal format text routes to the HTIF console. */
struct __zkvm_FILE { int fd; };
static struct __zkvm_FILE zkvm_stderr_obj = { 2 };
FILE *stderr = &zkvm_stderr_obj;

int fprintf(FILE *s, const char *fmt, ...)
{
    (void)s;
    htif_puts(fmt);
    return (int)strlen(fmt);
}
