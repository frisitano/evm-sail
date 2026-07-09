/* Self-contained native harness for the evm-sail model: the ONE host-side I/O
 * + run surface, linked into the ctypes .dylibs (build_lib.sh /
 * build_runner_lib.sh) AND the standalone zkvm_native exe (main.c CLI). Never
 * linked into the real zkVM guest, whose baked-vector / HTIF paths stay in
 * zkvm_input.c / zkvm_io.c.
 *
 * This file owns the WHOLE native I/O + harness surface so every host build
 * links exactly one file: the input buffer + ssz_src accessors, the emit_out
 * output buffer + accessors, the model lifecycle, the large-stack run_once, and
 * the between-fixtures clear_memory. It replaces the old split across
 * zkvm_input.c (-DERE_GUEST) + native_io.c + runner_ffi.c and their duplicated
 * ssz_src / double output buffer. See test_utils.h for the contract. */
#include "sail.h" /* unit / UNIT / bool / sail_int */
#include "test_utils.h"

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

/* ------------------------- input byte source ---------------------------- */
/* The host hands the guest its private input via evmsail_set_input; the Sail
 * SSZ/stream decoder reads it through ssz_src_* (io.sail). One buffer, indexed. */
static const unsigned char *g_in = NULL;
static unsigned long g_in_len = 0;

void evmsail_set_input(const unsigned char *p, unsigned long n)
{
    g_in = p;
    g_in_len = n;
}

uint64_t ssz_src_len(const unit u)
{
    (void)u;
    return (uint64_t)g_in_len;
}

uint64_t ssz_src_byte(sail_int idx)
{
    unsigned long i = mpz_get_ui(idx);
    return (i < g_in_len) ? (uint64_t)g_in[i] : 0;
}

const uint8_t *evmsail_ssz_ptr(uint64_t off, uint64_t len)
{
    uint64_t total = (uint64_t)g_in_len;
    if (off > total || len > total - off) return NULL;
    return g_in + off;
}

/* Bulk slice readers: read n (<=8) bytes in ONE FFI call instead of n crossings
 * -- the hot path of SSZ decoding (offsets, fields). */
uint64_t ssz_src_le(sail_int off, sail_int n) /* little-endian */
{
    unsigned long o = mpz_get_ui(off);
    long k = (long)mpz_get_ui(n);
    uint64_t v = 0;
    for (long i = 0; i < k; i++) {
        uint64_t b = (o + (unsigned long)i < g_in_len) ? g_in[o + i] : 0;
        v |= b << (8 * i);
    }
    return v;
}
uint64_t ssz_src_be(sail_int off, sail_int n) /* big-endian */
{
    unsigned long o = mpz_get_ui(off);
    long k = (long)mpz_get_ui(n);
    uint64_t v = 0;
    for (long i = 0; i < k; i++) {
        uint64_t b = (o + (unsigned long)i < g_in_len) ? g_in[o + i] : 0;
        v = (v << 8) | b;
    }
    return v;
}

/* ------------------------- output byte sink ----------------------------- */
/* The guest emits its canonical result byte-by-byte via emit_out -> el_emit_out
 * (io.sail / build.sail). Buffer it; the host reads the buffer after the run. */
static unsigned char g_out[1u << 17];
static size_t g_out_len = 0;

unit el_emit_out(uint64_t b)
{
    if (g_out_len < sizeof g_out) g_out[g_out_len++] = (unsigned char)(b & 0xff);
    return UNIT;
}

/* --------------------------- state resets ------------------------------- */
/* Reset surface -- all plain C FFI symbols (ffi/*.c), so the harness wipes state
 * WITHOUT any Sail-level reset function. This is the whole point: k_world_reset
 * is driver/test plumbing, not EVM semantics, so it does not belong in the
 * model. We call its constituent FFI resets directly instead. */
extern unit acct_wset_reset(unit);           /* block account write-set overlay */
extern unit storage_wset_reset(unit);        /* block storage write-set overlay */
extern unit bal_reset(unit);                 /* EIP-7928 block-access-list accumulator */
extern unit warm_reset(unit);                /* EIP-2929 warm sets */
extern unit transient_storage_reset(unit u); /* EIP-1153 transient storage */
extern unit logs_reset(unit);
extern unit selfdestr_reset(unit);
extern unit created_reset(unit);
extern unit journal_reset(unit);
extern unit nodedb_reset(unit);              /* hash-keyed witness node store */
extern unit code_db_reset(unit);             /* content-addressed code store (missing-code tests) */
extern bool have_exception;                  /* generated: a Sail throw escaped the run */

/* clear_memory: the C-side full world wipe that replaces k_world_reset. Zeroes
 * every piece of guest state that persists across in-process runs.
 *
 * k_refund (a sail_int register) is intentionally NOT touched: k_tx_reset already
 * zeroes it on every transaction, and a fixture always runs its txs before the
 * refund is read -- so a C-side sail_int poke would be redundant. */
void evmsail_clear_memory(void)
{
    /* block-level overlays -- the pieces per-tx k_tx_reset does NOT clear */
    acct_wset_reset(UNIT);
    storage_wset_reset(UNIT);
    bal_reset(UNIT);
    /* ephemeral per-frame/per-tx state (also cleared by k_tx_reset, but a
     * zero-transaction block would otherwise inherit it) */
    warm_reset(UNIT);
    transient_storage_reset(UNIT);
    logs_reset(UNIT);
    selfdestr_reset(UNIT);
    created_reset(UNIT);
    journal_reset(UNIT);
    /* content-addressed witness stores -- cleared so a fixture that
     * under-specifies its witness FAILS (valid=false) instead of borrowing a
     * stale node/code that an EARLIER fixture registered. The node DB alone is
     * not enough: the "codes_missing_*" negative tests need the code DB too. */
    nodedb_reset(UNIT);
    code_db_reset(UNIT);
    /* a Sail exception that propagated out of the previous run (runner: an
     * uncaught InvalidBlock, captured by the dump's ok flag). Left set it would
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

/* --------------------- post-run state snapshot (analysis) ---------------- */
/* A C-side dump of the model's live FFI state AFTER a run, marshalled into a
 * self-describing binary blob that Python (dump_state.py) decodes into native
 * types. This is an ANALYSIS surface, separate from the runner's gas+root output
 * -- it lets a mismatch be inspected without the model emitting anything extra.
 *
 * Content (all multi-byte ints big-endian):
 *   'G' ok[1] gas[8] root[32]        (the run RESULT -- the runner's ONLY
 *                                     output channel: ok = no uncaught Sail
 *                                     exception; gas = the zkvm_out_gas
 *                                     register; root = compute_state_root(),
 *                                     called on the live post-run state, zeros
 *                                     when ok=0)
 *       when ok=0, followed by the CAPTURED exception:
 *       err[1] loc_len[2] loc[loc_len]   (err = the BlockError enum value of
 *                                     the InvalidBlock that escaped, 0xff if
 *                                     none recorded; loc = the Sail source
 *                                     position of the throw site)
 *   'A' u32 n_acct  { hkey[32] nonce[8] bal[32] sroot[32] chash[32] base[1]
 *                     u32 n_slot { slot[32] val[32] }* }*
 *   'S' u32 depth   { word[32] }*   (stack, top-first)
 *   'M' u32 frame_depth              (EVM memory: only the call-frame depth is
 *                                     available C-side; the logical byte length
 *                                     is a Sail register, and for the runner
 *                                     memory is reset per tx -- empty post-run)
 *   'E'
 * The account/storage rows are the WRITE-SET UNION (what execution touched);
 * unchanged witness-base values are not enumerable here (that's what the root
 * commits to). */
extern uint64_t acct_wset_union_count(unit);
extern void     acct_wset_union_hkey(lbits *, uint64_t);
extern uint64_t acct_wset_union_nonce(uint64_t);
extern void     acct_wset_union_bal(lbits *, uint64_t);
extern void     acct_wset_union_sroot(lbits *, uint64_t);
extern void     acct_wset_union_chash(lbits *, uint64_t);
extern bool     acct_wset_union_base_exists(uint64_t);
extern uint64_t storage_wset_union_count(lbits);
extern void     storage_wset_union_slot(lbits *, lbits, uint64_t);
extern void     storage_wset_union_val(lbits *, lbits, uint64_t);
extern uint64_t hs_depth(unit);
extern void     hs_peek_w(lbits *, uint64_t);
extern uint64_t hm_depth(unit);

/* The run result, WITHOUT model-side plumbing: the per-entry gas register
 * (zkvm_out_gas), the model's OWN root computation called directly
 * (compute_state_root exists in both entries), and the generated exception
 * state (a Sail throw that propagated out of zmain -- e.g. the runner's
 * InvalidBlock on a malformed tx / deficient witness). */
extern sail_int zzzkvm_out_gas;
extern void     zcompute_state_root(lbits *, unit);
/* Mirror of the GENERATED exception representation (.build/zkvm_*.h): the
 * model's single constructor InvalidBlock(BlockError). The BlockError enum
 * values are fixed by declaration order in sail/exceptions.sail;
 * dump_state.py maps them back to names. throw_location carries the Sail
 * source position of the throw site. */
struct evmsail_exception { int kind; union { int block_error; } variants; };
extern struct evmsail_exception *current_exception;
extern char **throw_location;

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

unsigned long evmsail_dump_snapshot(const unsigned char **out)
{
    g_dump_len = 0;
    lbits w;

    d_byte('G');
    d_byte(have_exception ? 0 : 1);
    d_u64((uint64_t)mpz_get_ui(zzzkvm_out_gas));
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
        zcompute_state_root(&w, UNIT);
        d_word(&w);
    }

    d_byte('A');
    uint64_t na = acct_wset_union_count(UNIT);
    d_u32((uint32_t)na);
    for (uint64_t i = 0; i < na; i++) {
        acct_wset_union_hkey(&w, i); d_word(&w);       /* keccak(address) */
        lbits hk = w;                                   /* reuse as the storage account key */
        d_u64(acct_wset_union_nonce(i));
        acct_wset_union_bal(&w, i);   d_word(&w);
        acct_wset_union_sroot(&w, i); d_word(&w);
        acct_wset_union_chash(&w, i); d_word(&w);
        d_byte(acct_wset_union_base_exists(i) ? 1 : 0);
        uint64_t ns = storage_wset_union_count(hk);
        d_u32((uint32_t)ns);
        for (uint64_t j = 0; j < ns; j++) {
            storage_wset_union_slot(&w, hk, j); d_word(&w);
            storage_wset_union_val(&w, hk, j);  d_word(&w);
        }
    }

    d_byte('S');
    uint64_t sd = hs_depth(UNIT);
    d_u32((uint32_t)sd);
    for (uint64_t n = 0; n < sd; n++) { hs_peek_w(&w, n); d_word(&w); }

    d_byte('M');
    d_u32((uint32_t)hm_depth(UNIT));

    d_byte('E');
    *out = g_dump;
    return (unsigned long)g_dump_len;
}
