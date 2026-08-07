/* Native implementation of the same standard read_input/write_output ABI used
 * by the zkVM guest, plus the reusable test-process lifecycle and dump hooks. */
#include "region_access.h"
#include "exceptions.h"
#include "frame_stack.h"
#include EVMSAIL_MODEL_H
#include "value_convert.h"
#include "test_utils.h"
#include "zkvm_io.h"

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* The standard Sail RTS refers to these hooks, which a standalone generated
 * main normally owns.  Native model builds deliberately use --c-no-main and
 * provide their lifecycle here instead. */
void (*sail_rts_set_coverage_file)(const char *) = NULL;
void model_pre_exit(void) {}

/* --------------------------- standard I/O ------------------------------- */
static const uint8_t *g_in = NULL;
static size_t g_in_len = 0;

static void set_input(const uint8_t *p, size_t n)
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
extern unit warm_reset(uint64_t);            /* EIP-2929 warm sets */
extern unit transient_storage_reset(unit u); /* EIP-1153 transient storage */
extern unit logs_reset(unit);
extern unit state_journal_reset(unit);
extern unit nodedb_reset(unit);              /* hash-keyed witness node store */
extern unit code_db_reset(unit);             /* content-addressed code store (missing-code tests) */

/* clear_memory: the C-side full world wipe that replaces k_world_reset. Zeroes
 * every piece of guest state that persists across in-process runs. */
static void reset_world(void)
{
    fatal_error_reset();
    /* block-level overlays -- the pieces per-tx k_tx_reset does NOT clear */
    acct_db_reset(UNIT);
    storage_db_reset(UNIT);
    bal_reset(UNIT);
    /* ephemeral per-frame/per-tx state (also cleared by k_tx_reset, but a
     * zero-transaction block would otherwise inherit it) */
    warm_reset(1);
    transient_storage_reset(UNIT);
    logs_reset(UNIT);
    state_journal_reset(UNIT);
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

void guest_reset(void)
{
    reset_world();
    g_out_len = 0;
}

/* --------------------------- lifecycle + run ---------------------------- */
extern void model_init(void);
extern void model_fini(void);

void guest_init(void) { model_init(); }
void guest_fini(void) { model_fini(); }

/* The spec backend has no optimized-invariant boundary, but still reports
 * native thread setup failures through the common harness ABI. */
static char last_error[1024];

static void record_thread_error(const char *operation, int error)
{
    (void)snprintf(last_error, sizeof last_error, "%s failed: %s",
                   operation, strerror(error));
}

const char *guest_last_error(void) { return last_error; }

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

unsigned long guest_run(const unsigned char *in, unsigned long n,
                        const unsigned char **out)
{
    g_out_len = 0;
    last_error[0] = '\0';
    set_input(in, n);

    pthread_attr_t attr;
    int error = pthread_attr_init(&attr);
    if (error != 0) {
        record_thread_error("pthread_attr_init", error);
        *out = g_out;
        return 0;
    }

    error = pthread_attr_setstacksize(&attr, GUEST_STACK_BYTES);
    if (error != 0) {
        record_thread_error("pthread_attr_setstacksize", error);
        (void)pthread_attr_destroy(&attr);
        *out = g_out;
        return 0;
    }

    pthread_t t;
    error = pthread_create(&t, &attr, run_thread, NULL);
    (void)pthread_attr_destroy(&attr);
    if (error != 0) {
        record_thread_error("pthread_create", error);
        *out = g_out;
        return 0;
    }

    error = pthread_join(t, NULL);
    if (error != 0) record_thread_error("pthread_join", error);

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
 *       err[1] loc_len[2] loc[loc_len]   (err = the FatalError enum value of
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
extern u256 acct_dump_hkey(uint64_t);
extern fixed_bytes_20 acct_dump_address(uint64_t);
extern uint64_t acct_dump_nonce(uint64_t);
extern u256 acct_dump_balance(uint64_t);
extern u256 acct_dump_storage_root(uint64_t);
extern u256 acct_dump_code_hash(uint64_t);
extern uint64_t storage_dump_count(u256);
extern u256 storage_dump_slot(u256, uint64_t);
extern u256 storage_dump_value(u256, uint64_t);
extern uint64_t stack_depth(unit);
extern u256 stack_peek_word(uint64_t);
extern uint64_t hm_depth(unit);

static unsigned char g_dump[1u << 22];
static size_t g_dump_len;
static char g_validation_location[513];

static void d_byte(unsigned char b) { if (g_dump_len < sizeof g_dump) g_dump[g_dump_len++] = b; }
static void d_u32(uint32_t v) { for (int i = 3; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
static void d_u64(uint64_t v) { for (int i = 7; i >= 0; i--) d_byte((unsigned char)(v >> (8 * i))); }
static void d_word(u256 value) {
    uint64_t words[4];
    sail_word_to_be_words4(words, value);
    for (int i = 0; i < 4; i++) {
        uint64_t limb = words[i];
        for (int j = 0; j < 8; j++) d_byte((unsigned char)(limb >> (56 - 8 * j)));
    }
}

static void d_address(fixed_bytes_20 value) {
    uint8_t bytes[20];
    evmsail_address_to_be_bytes(bytes, value);
    for (size_t i = 0; i < sizeof bytes; i++) d_byte(bytes[i]);
}

static void d_hash(fixed_bytes_32 value)
{
    uint8_t bytes[32];
    evmsail_hash_to_be_bytes(bytes, value);
    for (size_t i = 0; i < sizeof bytes; i++) d_byte(bytes[i]);
}

static fixed_bytes_32 model_state_root(void)
{
    return zcompute_state_root(UNIT);
}

static fixed_bytes_32 model_account_storage_root(fixed_bytes_20 address)
{
    return zdebug_account_storage_root(address);
}

static void dispose_hash(fixed_bytes_32 *value)
{
    (void)value;
}

unsigned long guest_debug_dump(const unsigned char **out)
{
    g_dump_len = 0;
    fixed_bytes_32 root = {0};
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

    d_byte('B');
    d_byte(zvalidation_debug_gas_present ? 1 : 0);
    if (zvalidation_debug_gas_present) {
        d_u64(sail_int_get_ui(zvalidation_debug_header_gas_actual));
        d_u64(sail_int_get_ui(zvalidation_debug_header_gas_expected));
        d_u64(sail_int_get_ui(zvalidation_debug_execution_gas));
        d_u64(sail_int_get_ui(zvalidation_debug_state_gas));
    }

    d_byte('A');
    uint64_t na = state_available ? acct_dump_count(UNIT) : 0;
    d_u32((uint32_t)na);
    for (uint64_t i = 0; i < na; i++) {
        u256 hk = acct_dump_hkey(i);
        d_word(hk);                              /* keccak(address) */
        fixed_bytes_20 addr = acct_dump_address(i);
        d_address(addr);
        d_u64(acct_dump_nonce(i));
        d_word(acct_dump_balance(i));
        d_word(acct_dump_storage_root(i));
        fixed_bytes_32 storage_root = model_account_storage_root(addr);
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
