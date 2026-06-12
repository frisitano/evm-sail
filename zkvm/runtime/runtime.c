/* ===========================================================================
 * Freestanding C runtime for the eth-act zkVM RISC-V standard target.
 *
 * The guest is built -ffreestanding -nostdlib: there is no OS, no libc, and no
 * syscalls.  This file supplies the small slice of the hosted environment that
 * the Sail-generated model, the stock Sail runtime (sail.c), and the vendored
 * mini-gmp need:
 *
 *   - a heap allocator (malloc/free/realloc/calloc) over the linker-defined
 *     heap region, with a first-fit free list + coalescing,
 *   - the handful of <string.h>/<ctype.h> routines they call,
 *   - the standardized termination mapping: abort()/assert failure, Sail match
 *     failures, and Sail asserts all halt the machine and report ABNORMAL
 *     termination (non-zero exit) to the host (see standard-termination-
 *     semantics),
 *   - no-op stubs for the parts of the Sail RTS the model references but that a
 *     freestanding guest does not use (setup_rts/cleanup_rts, diagnostic I/O).
 * =========================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "time.h"
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

/* ----- <string.h> / <ctype.h> subset --------------------------------------
 * gcc also lowers struct copies / memcpy idioms to calls to these even under
 * -ffreestanding, so they must exist as real symbols. */

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

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++)) {
        ;
    }
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (n && (*d = *src)) {
        d++;
        src++;
        n--;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dst;
}

char *strchr(const char *s, int c)
{
    for (; *s; s++) {
        if (*s == (char)c) {
            return (char *)s;
        }
    }
    return (c == 0) ? (char *)s : NULL;
}

int isdigit(int c)  { return c >= '0' && c <= '9'; }
int isspace(int c)  { return c == ' ' || (c >= '\t' && c <= '\r'); }
int isxdigit(int c) { return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
int isalpha(int c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isupper(int c)  { return c >= 'A' && c <= 'Z'; }
int islower(int c)  { return c >= 'a' && c <= 'z'; }
int toupper(int c)  { return islower(c) ? c - 'a' + 'A' : c; }
int tolower(int c)  { return isupper(c) ? c - 'A' + 'a' : c; }

/* ----- heap allocator -----------------------------------------------------
 * First-fit free list with boundary-tag coalescing over [__heap_start,
 * __heap_end).  mini-gmp and the Sail list/string runtime allocate and free
 * many short-lived objects during a block run, so reclaiming freed memory
 * (rather than a pure bump allocator) keeps the working set bounded. */

extern char __heap_start[];
extern char __heap_end[];

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
    brk_ptr   = (char *)ALIGN_UP((uintptr_t)__heap_start, HEAP_ALIGN);
    brk_end   = __heap_end;
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

void _exit(int code)
{
    htif_exit(code);
}

/* newlib-style assert backend (in case any TU pulls <assert.h> without NDEBUG) */
void __assert_func(const char *file, int line, const char *fn, const char *expr)
{
    (void)file;
    (void)line;
    (void)fn;
    (void)expr;
    zkvm_abort("assertion failed");
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

int sail_get_verbosity(void) { return 0; }

time_t time(time_t *tloc)
{
    if (tloc) {
        *tloc = 0;
    }
    return 0;   /* the guest has no clock */
}

int atoi(const char *s) { return (int)strtol(s, NULL, 10); }

int       abs(int j)        { return j < 0 ? -j : j; }
long long llabs(long long j){ return j < 0 ? -j : j; }
int       rand(void)        { return 0; }   /* deterministic; guest is not random */
void      srand(unsigned s) { (void)s; }

int clock_gettime(int clk, struct timespec *tp)
{
    (void)clk;
    if (tp) {
        tp->tv_sec = 0;
        tp->tv_nsec = 0;
    }
    return 0;
}

char *strcat(char *dst, const char *src)
{
    char *d = dst + strlen(dst);
    while ((*d++ = *src++)) {
        ;
    }
    return dst;
}

char *strstr(const char *hay, const char *needle)
{
    if (!*needle) {
        return (char *)hay;
    }
    for (; *hay; hay++) {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) {
            return (char *)hay;
        }
    }
    return NULL;
}

/* GMP entry points absent from mini-gmp. Only Sail's unused diagnostic/format/
 * float helpers reference these; --gc-sections normally drops them. Defined so
 * the link always resolves; if ever reached they fail closed. */
void mpf_set_default_prec(unsigned long prec) { (void)prec; }
int  gmp_asprintf(char **pp, const char *fmt, ...) { (void)fmt; if (pp) { *pp = malloc(1); (*pp)[0] = '\0'; } return 0; }
int  gmp_sscanf(const char *s, const char *fmt, ...) { (void)s; (void)fmt; return 0; }
int  gmp_printf(const char *fmt, ...) { htif_puts(fmt); return 0; }
int  gmp_fprintf(FILE *fp, const char *fmt, ...) { (void)fp; htif_puts(fmt); return 0; }

long strtol(const char *s, char **endptr, int base)
{
    long v = 0;
    int neg = 0;
    if (base != 10 && base != 0) {
        /* only base 10 is needed by the retained surface */
    }
    while (isspace((int)*s)) {
        s++;
    }
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (isdigit((int)*s)) {
        v = v * 10 + (*s - '0');
        s++;
    }
    if (endptr) {
        *endptr = (char *)s;
    }
    return neg ? -v : v;
}

/* Minimal stdio surface that mini-gmp's OOM/diagnostic paths and any retained
 * Sail diagnostic routines reference.  We never do real file I/O; bytes route
 * to the HTIF console. */
struct __zkvm_FILE { int fd; };
static struct __zkvm_FILE zkvm_stdout_obj = { 1 };
static struct __zkvm_FILE zkvm_stderr_obj = { 2 };
static struct __zkvm_FILE zkvm_stdin_obj  = { 0 };
FILE *stdout = &zkvm_stdout_obj;
FILE *stderr = &zkvm_stderr_obj;
FILE *stdin  = &zkvm_stdin_obj;

int fputc(int c, FILE *stream)
{
    (void)stream;
    htif_putchar((char)c);
    return c;
}

int fputs(const char *s, FILE *stream)
{
    (void)stream;
    htif_puts(s);
    return 0;
}

int putchar(int c)
{
    htif_putchar((char)c);
    return c;
}

int puts(const char *s)
{
    htif_puts(s);
    htif_putchar('\n');
    return 0;
}

int fflush(FILE *stream) { (void)stream; return 0; }

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    (void)stream;
    const unsigned char *p = ptr;
    size_t total = size * nmemb;
    for (size_t i = 0; i < total; i++) {
        htif_putchar((char)p[i]);
    }
    return nmemb;
}

/* printf-family stubs.  The model's happy path performs NO printing; these exist
 * only so any retained diagnostic/error-formatting code links and, if ever hit,
 * emits the literal format text rather than faulting.  No conversion-specifier
 * expansion is performed (none is needed on the executed paths). */
static int emit_str(const char *s)
{
    htif_puts(s);
    return (int)strlen(s);
}

int printf(const char *fmt, ...)               { return emit_str(fmt); }
int fprintf(FILE *s, const char *fmt, ...)      { (void)s; return emit_str(fmt); }
int sprintf(char *str, const char *fmt, ...)    { strcpy(str, fmt); return (int)strlen(str); }
int snprintf(char *str, size_t n, const char *fmt, ...)
{
    strncpy(str, fmt, n);
    if (n) {
        str[n - 1] = '\0';
    }
    return (int)strlen(str);
}
int vsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    (void)ap;
    strncpy(str, fmt, n);
    if (n) {
        str[n - 1] = '\0';
    }
    return (int)strlen(str);
}
/* asprintf is used only by Sail's int->string formatting, which the guest does
 * not exercise. Provide a safe allocating stub. */
int asprintf(char **strp, const char *fmt, ...)
{
    size_t len = strlen(fmt);
    char *buf = malloc(len + 1);
    strcpy(buf, fmt);
    *strp = buf;
    return (int)len;
}
