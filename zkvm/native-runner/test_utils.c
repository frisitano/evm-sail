/* Native implementation of the same standard read_input/write_output ABI used
 * by the zkVM guest, plus the reusable test-process lifecycle and dump hooks. */
#include "byte_slice_glue.h"
#include "sail.h" /* unit / UNIT / bool */
#include "test_utils.h"
#include "zkvm_io.h"

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* --------------------------- standard I/O ------------------------------- */
static const uint8_t *g_in = NULL;
static size_t g_in_len = 0;

static void evmsail_set_input(const uint8_t *p, size_t n)
{
    g_in = p;
    g_in_len = n;
    evmsail_input_reset();
}

void read_input(const uint8_t **buf_ptr, size_t *buf_size)
{
    *buf_ptr = g_in;
    *buf_size = g_in_len;
}

static unsigned char g_out[1u << 17];
static size_t g_out_len = 0;

void write_output(const uint8_t *output, size_t size)
{
    size_t available = sizeof g_out - g_out_len;
    if (size > available) size = available;
    if (size != 0) memcpy(g_out + g_out_len, output, size);
    g_out_len += size;
}

/* --------------------------- state resets ------------------------------- */
/* Reset surface -- all plain C FFI symbols, so the harness wipes state
 * WITHOUT any Sail-level reset function. This is the whole point: k_world_reset
 * is driver/test plumbing, not EVM semantics, so it does not belong in the
 * model. We call its constituent FFI resets directly instead. */
extern unit acct_db_reset(unit);             /* transaction + block account state */
extern unit storage_db_reset(unit);          /* transaction + block storage state */
extern unit bal_reset(unit);                 /* EIP-7928 block-access-list accumulator */
extern unit warm_reset(unit);                /* EIP-2929 warm sets */
extern unit transient_storage_reset(unit u); /* EIP-1153 transient storage */
extern unit logs_reset(unit);
extern unit journal_reset(unit);
extern unit nodedb_reset(unit);              /* hash-keyed witness node store */
extern unit code_db_reset(unit);             /* content-addressed code store (missing-code tests) */
extern bool have_exception;                  /* generated: a Sail throw escaped the run */

/* clear_memory: the C-side full world wipe that replaces k_world_reset. Zeroes
 * every piece of guest state that persists across in-process runs. */
void evmsail_clear_memory(void)
{
    /* block-level overlays -- the pieces per-tx k_tx_reset does NOT clear */
    acct_db_reset(UNIT);
    storage_db_reset(UNIT);
    bal_reset(UNIT);
    /* ephemeral per-frame/per-tx state (also cleared by k_tx_reset, but a
     * zero-transaction block would otherwise inherit it) */
    warm_reset(UNIT);
    transient_storage_reset(UNIT);
    logs_reset(UNIT);
    journal_reset(UNIT);
    /* content-addressed witness stores -- cleared so a fixture that
     * under-specifies its witness FAILS (valid=false) instead of borrowing a
     * stale node/code that an EARLIER fixture registered. The node DB alone is
     * not enough: the "codes_missing_*" negative tests need the code DB too. */
    nodedb_reset(UNIT);
    code_db_reset(UNIT);
    /* a Sail exception that propagated out of the previous run. Left set it would
     * poison the next run: generated code short-circuits every call while
     * have_exception is true. */
    have_exception = false;
}

void evmsail_test_reset(void)
{
    evmsail_clear_memory();
    g_out_len = 0;
}

/* --------------------------- lifecycle + run ---------------------------- */
extern unit zmain(unit); /* model entry (Sail-generated) */
extern void model_init(void);
extern void model_fini(void);

void evmsail_lib_init(void) { model_init(); }
void evmsail_lib_fini(void) { model_fini(); }

/* Big stack: the guest recurses over multi-MB SSZ lists; the exe pins a 512 MB
 * main-thread stack (build.sh). In-process the ctypes caller's thread stack is
 * far smaller, so run the guest on a dedicated large-stack thread. */
#define GUEST_STACK_BYTES ((size_t)0x20000000)

static void *run_thread(void *unused)
{
    (void)unused;
    (void)zmain(UNIT);
    return NULL;
}

unsigned long evmsail_run_once(const unsigned char *in, unsigned long n,
                               const unsigned char **out)
{
    g_out_len = 0;
    evmsail_set_input(in, n);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, GUEST_STACK_BYTES);
    pthread_t t;
    if (pthread_create(&t, &attr, run_thread, NULL) == 0) {
        pthread_join(t, NULL);
    } else {
        /* fallback: run inline (may overflow on deep inputs, but never silently
         * skips) */
        (void)run_thread(NULL);
    }
    pthread_attr_destroy(&attr);

    *out = g_out;
    return (unsigned long)g_out_len;
}

/* ------------------------- debug state dump ------------------------------ */
/* An on-demand C-side dump of the model's live FFI state AFTER a run,
 * marshalled into a
 * self-describing binary blob that Python (dump_state.py) decodes into native
 * types. This is a native test utility, never linked into the real guest, and
 * normal fixture runs do not invoke it.
 *
 * Content (all multi-byte ints big-endian):
 *   'G' ok[1] root[32] rebuilt[32]   (root = compute_state_root() over the live
 *                                     post-run state; rebuilt recomputes the
 *                                     same materialized state from an empty
 *                                     trie; both are zero when ok=0)
 *       when ok=0, followed by the CAPTURED exception:
 *       err[1] loc_len[2] loc[loc_len]   (err = the BlockError enum value of
 *                                     the InvalidBlock that escaped, 0xff if
 *                                     none recorded; loc = the Sail source
 *                                     position of the throw site)
 *   'O' u32 len output[len]          (the canonical output emitted by main.sail)
 *   'V' failed[1]                    (main.sail's caught validation failure)
 *       when failed=1: scope[1] reason[1]
 *   'A' u32 n_acct  { hkey[32] address[20] nonce[8] bal[32]
 *                     base_sroot[32] computed_sroot[32] chash[32]
 *                     u32 n_slot { slot[32] val[32] }* }*
 *   'S' u32 depth   { word[32] }*   (stack, top-first)
 *   'M' u32 frame_depth              (EVM memory: only the call-frame depth is
 *                                     available C-side; the logical byte length
 *                                     is a Sail register, and after a block
 *                                     memory is reset per tx -- empty post-run)
 *   'E'
 * The account/storage entries are the cumulative state touched by execution;
 * unchanged authenticated-base values are not enumerable here. */
extern uint64_t acct_dump_count(unit);
extern void     acct_dump_hkey(lbits *, uint64_t);
extern void     acct_dump_address(lbits *, uint64_t);
extern uint64_t acct_dump_nonce(uint64_t);
extern void     acct_dump_balance(lbits *, uint64_t);
extern void     acct_dump_storage_root(lbits *, uint64_t);
extern void     acct_dump_code_hash(lbits *, uint64_t);
extern uint64_t storage_dump_count(lbits);
extern void     storage_dump_slot(lbits *, lbits, uint64_t);
extern void     storage_dump_value(lbits *, lbits, uint64_t);
extern uint64_t stack_depth(unit);
extern void     stack_peek_word(lbits *, uint64_t);
extern uint64_t hm_depth(unit);

/* The model's own root computation and generated exception state. */
extern void     zcompute_state_root(lbits *, unit);
extern void     zdebug_account_storage_root(lbits *, lbits);
extern void     zdebug_rebuild_state_root(lbits *, unit);
/* Mirror of the GENERATED exception representation (.build/zkvm_*.h): the
 * model's single constructor InvalidBlock(BlockError). The BlockError enum
 * values are fixed by declaration order in sail/exceptions.sail;
 * dump_state.py maps them back to names. throw_location carries the Sail
 * source position of the throw site. */
struct evmsail_exception { int kind; union { int block_error; } variants; };
extern struct evmsail_exception *current_exception;
extern char **throw_location;
extern bool zvalidation_failure_present;
extern uint64_t zvalidation_failure_scope;
extern int zvalidation_failure_reason;

static unsigned char g_dump[1u << 22];
static size_t g_dump_len;

static void d_byte(unsigned char b) { if (g_dump_len < sizeof g_dump) g_dump[g_dump_len++] = b; }
static void d_u32(uint32_t v) { for (int i = 3; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
static void d_u64(uint64_t v) { for (int i = 7; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
/* a 256-bit lbits (d[0]=LSBs, d[3]=MSBs) as 32 big-endian bytes */
static void d_word(const lbits *v) {
    for (int i = 0; i < 4; i++) {
        uint64_t limb = v->d[3 - i];
        for (int j = 0; j < 8; j++) d_byte((unsigned char)(limb >> (56 - 8 * j)));
    }
}

unsigned long evmsail_debug_dump(const unsigned char **out)
{
    g_dump_len = 0;
    lbits w, rebuilt;

    if (!have_exception) {
        zcompute_state_root(&w, UNIT);
        zdebug_rebuild_state_root(&rebuilt, UNIT);
    }
    d_byte('G');
    d_byte(have_exception ? 0 : 1);
    if (have_exception) {
        for (int i = 0; i < 32; i++) d_byte(0);
        d_byte(current_exception
                   ? (unsigned char)current_exception->variants.block_error
                   : 0xff);
        const char *loc =
            (throw_location && *throw_location) ? *throw_location : "";
        size_t ln = 0;
        while (loc[ln] && ln < 512) ln++;
        d_byte((unsigned char)(ln >> 8));
        d_byte((unsigned char)(ln & 0xff));
        for (size_t i = 0; i < ln; i++) d_byte((unsigned char)loc[i]);
    } else {
        d_word(&w);
    }
    if (have_exception) {
        for (int i = 0; i < 32; i++) d_byte(0);
    } else {
        d_word(&rebuilt);
    }

    d_byte('O');
    d_u32((uint32_t)g_out_len);
    for (size_t i = 0; i < g_out_len; i++) d_byte(g_out[i]);

    d_byte('V');
    d_byte(zvalidation_failure_present ? 1 : 0);
    if (zvalidation_failure_present) {
        d_byte((unsigned char)zvalidation_failure_scope);
        d_byte((unsigned char)zvalidation_failure_reason);
    }

    d_byte('A');
    uint64_t na = acct_dump_count(UNIT);
    d_u32((uint32_t)na);
    for (uint64_t i = 0; i < na; i++) {
        acct_dump_hkey(&w, i); d_word(&w);       /* keccak(address) */
        lbits hk = w;                                   /* reuse as the storage account key */
        lbits addr;
        acct_dump_address(&addr, i);
        for (int j = 0; j < 20; j++) {
            unsigned shift = (unsigned)(19 - j) * 8u;
            d_byte((unsigned char)(addr.d[shift / 64u] >> (shift % 64u)));
        }
        d_u64(acct_dump_nonce(i));
        acct_dump_balance(&w, i);   d_word(&w);
        acct_dump_storage_root(&w, i); d_word(&w);
        zdebug_account_storage_root(&w, addr); d_word(&w);
        acct_dump_code_hash(&w, i); d_word(&w);
        uint64_t ns = storage_dump_count(hk);
        d_u32((uint32_t)ns);
        for (uint64_t j = 0; j < ns; j++) {
            storage_dump_slot(&w, hk, j); d_word(&w);
            storage_dump_value(&w, hk, j);  d_word(&w);
        }
    }

    d_byte('S');
    uint64_t sd = stack_depth(UNIT);
    d_u32((uint32_t)sd);
    for (uint64_t n = 0; n < sd; n++) { stack_peek_word(&w, n); d_word(&w); }

    d_byte('M');
    d_u32((uint32_t)hm_depth(UNIT));

    d_byte('E');
    *out = g_dump;
    return (unsigned long)g_dump_len;
}
