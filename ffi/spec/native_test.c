/* Native implementation of the same standard read_input/write_output ABI used
 * by the zkVM guest, plus the reusable test-process lifecycle and dump hooks. */
#include "region_access.h"
#include "frame_stack.h"
#include EVMSAIL_MODEL_H
#include "value_convert.h"
#include "test_utils.h"
#include "zkvm_io.h"

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* The standard Sail RTS refers to these hooks, which a standalone generated
 * main normally owns.  Native model builds deliberately use --c-no-main and
 * provide their lifecycle here instead. */
void (*sail_rts_set_coverage_file)(const char *) = NULL;
void model_pre_exit(void) {}

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
extern unit host_state_checkpoint_reset(unit);
extern unit nodedb_reset(unit);              /* hash-keyed witness node store */
extern unit code_db_reset(unit);             /* content-addressed code store (missing-code tests) */

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
    host_state_checkpoint_reset(UNIT);
    /* content-addressed witness stores -- cleared so a fixture that
     * under-specifies its witness FAILS (valid=false) instead of borrowing a
     * stale node/code that an EARLIER fixture registered. The node DB alone is
     * not enough: the "codes_missing_*" negative tests need the code DB too. */
    nodedb_reset(UNIT);
    code_db_reset(UNIT);
    frame_stack_reset(UNIT);
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
 *   'G' ok[1] root[32]               (root = compute_state_root() over the live
 *                                     post-run state; zero after an uncaught
 *                                     Sail exception)
 *       when ok=0, followed by the CAPTURED exception:
 *       err[1] loc_len[2] loc[loc_len]   (err = the BlockError enum value of
 *                                     the InvalidBlock that escaped, 0xff if
 *                                     none recorded; loc = the Sail source
 *                                     position of the throw site)
 *   'O' u32 len output[len]          (the canonical output emitted by main.sail)
 *   'V' failed[1]                    (main.sail's caught validation failure)
 *       when failed=1: scope[1] reason[1] loc_len[2] loc[loc_len]
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
extern sail_u256 acct_dump_hkey(uint64_t);
extern sail_fixed_bytes_20 acct_dump_address(uint64_t);
extern uint64_t acct_dump_nonce(uint64_t);
extern sail_u256 acct_dump_balance(uint64_t);
extern sail_u256 acct_dump_storage_root(uint64_t);
extern sail_u256 acct_dump_code_hash(uint64_t);
extern uint64_t storage_dump_count(sail_u256);
extern sail_u256 storage_dump_slot(sail_u256, uint64_t);
extern sail_u256 storage_dump_value(sail_u256, uint64_t);
extern uint64_t stack_depth(unit);
extern sail_u256 stack_peek_word(uint64_t);
extern uint64_t hm_depth(unit);

static unsigned char g_dump[1u << 22];
static size_t g_dump_len;
static char g_validation_location[513];

unit validation_debug_capture_location(unit u)
{
    (void)u;
    const char *loc =
        (throw_location && *throw_location) ? *throw_location : "";
    size_t len = 0;
    while (loc[len] && len < sizeof g_validation_location - 1) {
        g_validation_location[len] = loc[len];
        len++;
    }
    g_validation_location[len] = '\0';
    return UNIT;
}

static void d_byte(unsigned char b) { if (g_dump_len < sizeof g_dump) g_dump[g_dump_len++] = b; }
static void d_u32(uint32_t v) { for (int i = 3; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
static void d_u64(uint64_t v) { for (int i = 7; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
static void d_word(sail_u256 value) {
    uint64_t words[4];
    sail_word_to_be_words4(words, value);
    for (int i = 0; i < 4; i++) {
        uint64_t limb = words[i];
        for (int j = 0; j < 8; j++) d_byte((unsigned char)(limb >> (56 - 8 * j)));
    }
}

static void d_address(sail_fixed_bytes_20 value) {
    uint8_t bytes[20];
    evmsail_address_to_be_bytes(bytes, value);
    for (size_t i = 0; i < sizeof bytes; i++) d_byte(bytes[i]);
}

static void d_hash(sail_fixed_bytes_32 value)
{
    uint8_t bytes[32];
    evmsail_hash_to_be_bytes(bytes, value);
    for (size_t i = 0; i < sizeof bytes; i++) d_byte(bytes[i]);
}

static sail_fixed_bytes_32 model_state_root(void)
{
    return zcompute_state_root(UNIT);
}

static sail_fixed_bytes_32 model_account_storage_root(sail_fixed_bytes_20 address)
{
    return zdebug_account_storage_root(address);
}

static void dispose_hash(sail_fixed_bytes_32 *value)
{
    (void)value;
}

unsigned long evmsail_debug_dump(const unsigned char **out)
{
    g_dump_len = 0;
    sail_fixed_bytes_32 root = {0};
    bool validation_failed = zvalidation_failure_present;
    bool state_available = !have_exception;
    const char *validation_loc =
        validation_failed ? g_validation_location : "";

    if (state_available) {
        root = model_state_root();
        state_available = !have_exception;
    }
    d_byte('G');
    d_byte(state_available ? 1 : 0);
    if (!state_available) {
        for (int i = 0; i < 32; i++) d_byte(0);
        d_byte(have_exception && current_exception
                   ? (unsigned char)current_exception->variants.zInvalidBlock
                   : validation_failed
                         ? (unsigned char)zvalidation_failure_reason
                         : 0xff);
        const char *loc =
            (have_exception && throw_location && *throw_location)
                ? *throw_location
                : "";
        size_t ln = 0;
        while (loc[ln] && ln < 512) ln++;
        d_byte((unsigned char)(ln >> 8));
        d_byte((unsigned char)(ln & 0xff));
        for (size_t i = 0; i < ln; i++) d_byte((unsigned char)loc[i]);
    } else {
        d_hash(root);
    }
    dispose_hash(&root);

    d_byte('O');
    d_u32((uint32_t)g_out_len);
    for (size_t i = 0; i < g_out_len; i++) d_byte(g_out[i]);

    d_byte('V');
    d_byte(zvalidation_failure_present ? 1 : 0);
    if (zvalidation_failure_present) {
        d_byte((unsigned char)zvalidation_failure_scope);
        d_byte((unsigned char)zvalidation_failure_reason);
        size_t ln = 0;
        while (validation_loc[ln] && ln < 512) ln++;
        d_byte((unsigned char)(ln >> 8));
        d_byte((unsigned char)(ln & 0xff));
        for (size_t i = 0; i < ln; i++)
            d_byte((unsigned char)validation_loc[i]);
    }

    d_byte('A');
    uint64_t na = state_available ? acct_dump_count(UNIT) : 0;
    d_u32((uint32_t)na);
    for (uint64_t i = 0; i < na; i++) {
        sail_u256 hk = acct_dump_hkey(i);
        d_word(hk);                              /* keccak(address) */
        sail_fixed_bytes_20 addr = acct_dump_address(i);
        d_address(addr);
        d_u64(acct_dump_nonce(i));
        d_word(acct_dump_balance(i));
        d_word(acct_dump_storage_root(i));
        sail_fixed_bytes_32 storage_root = model_account_storage_root(addr);
        if (have_exception) {
            for (int j = 0; j < 32; j++) d_byte(0);
        } else {
            d_hash(storage_root);
        }
        dispose_hash(&storage_root);
        d_word(acct_dump_code_hash(i));
        uint64_t ns = storage_dump_count(hk);
        d_u32((uint32_t)ns);
        for (uint64_t j = 0; j < ns; j++) {
            d_word(storage_dump_slot(hk, j));
            d_word(storage_dump_value(hk, j));
        }
    }

    d_byte('S');
    uint64_t sd = state_available ? stack_depth(UNIT) : 0;
    d_u32((uint32_t)sd);
    for (uint64_t n = 0; n < sd; n++) d_word(stack_peek_word(n));

    d_byte('M');
    d_u32(state_available ? (uint32_t)hm_depth(UNIT) : 0);

    d_byte('E');
    *out = g_dump;
    return (unsigned long)g_dump_len;
}
