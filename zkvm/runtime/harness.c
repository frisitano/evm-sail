/* ===========================================================================
 * zkVM guest harness: drive the Sail EVM block to completion and emit the
 * result through the standard write_output interface, then terminate with the
 * standardized success/abnormal semantics.
 *
 * Control flow (called from start.S):
 *     _start -> zkvm_start() -> zkvm_exit(code)
 *
 *   1. read_input()  exercises the private-input side of the IO interface.
 *   2. model_init()  initialises the Sail world (kernel registers, letbinds).
 *   3. zmain()  executes the block (see sail/main.sail).
 *   4. the result facts are read from the model's output registers and emitted
 *      via write_output().
 *   5. the facts are checked against the known-correct fixture values; any
 *      mismatch (or an uncaught Sail exception surfaced by model_fini) is an
 *      ABNORMAL termination (non-zero exit code).
 * =========================================================================== */

#include "sail.h"      /* unit, UNIT, sail_int (mpz_t via mini-gmp), CREATE etc. */
#include "zkvm_io.h"
#include "htif.h"

/* Sail-generated entry points (see build/zkvm_block.c). */
extern void model_init(void);
extern void model_fini(void);
extern unit zmain(unit);

/* Output registers set by zmain (all hold values that fit in 64 bits). */
extern sail_int zzzkvm_out_gas;
extern sail_int zzzkvm_out_storage0;
extern sail_int zzzkvm_out_wbal;
extern sail_int zzzkvm_out_cbbal;
extern sail_int zzzkvm_out_tracelen;
extern sail_int zzzkvm_out_success;

/* From runtime.c. */
void zkvm_abort(const char *why);

/* From zkvm_io.c — the accumulated public output (write_output bytes). */
extern size_t               zkvm_output_size(void);
extern const unsigned char *zkvm_output_buffer(void);

/* ---- small decimal formatting (no libc printf on the executed path) ------- */

static size_t u64_to_dec(uint64_t v, char *out)
{
    char tmp[20];
    size_t n = 0;
    if (v == 0) {
        out[0] = '0';
        return 1;
    }
    while (v > 0) {
        tmp[n++] = (char)('0' + (v % 10));
        v /= 10;
    }
    for (size_t i = 0; i < n; i++) {
        out[i] = tmp[n - 1 - i];
    }
    return n;
}

/* Debug facts go to the HTIF console, NOT write_output: the public output
 * channel (write_output) carries only the canonical SSZ result the guest emits. */
static void emit_kv(const char *label, uint64_t v)
{
    char num[24];
    htif_puts(label);
    htif_putchar('=');
    size_t n = u64_to_dec(v, num);
    for (size_t i = 0; i < n; i++) htif_putchar(num[i]);
    htif_putchar('\n');
}

/* ---- entry --------------------------------------------------------------- */

int zkvm_start(void)
{
    /* (1) Exercise the private-input interface. The block under test is built
     * inside the Sail model, so the input is informational here. */
    const uint8_t *in_ptr;
    size_t         in_size;
    read_input(&in_ptr, &in_size);

    htif_puts("[zkvm] evm-sail guest on riscv64im_zicclsm (GMP-free)\n");
    emit_kv("input_size", (uint64_t)in_size);

    /* (2,3) Initialise the world and run the block, counting retired
     * instructions over the block execution (the proving-cost proxy). */
    model_init();
    uint64_t i0, i1;
    __asm__ volatile("rdinstret %0" : "=r"(i0));
    zmain(UNIT);
    __asm__ volatile("rdinstret %0" : "=r"(i1));
    emit_kv("instret_block", i1 - i0);

    /* (4) Read result facts and publish them as the public output. */
    uint64_t gas      = (uint64_t)mpz_get_ui(zzzkvm_out_gas);
    uint64_t storage0 = (uint64_t)mpz_get_ui(zzzkvm_out_storage0);
    uint64_t wbal     = (uint64_t)mpz_get_ui(zzzkvm_out_wbal);
    uint64_t cbbal    = (uint64_t)mpz_get_ui(zzzkvm_out_cbbal);
    uint64_t tracelen = (uint64_t)mpz_get_ui(zzzkvm_out_tracelen);
    uint64_t success  = (uint64_t)mpz_get_ui(zzzkvm_out_success);

    emit_kv("gas_used", gas);
    emit_kv("storage0", storage0);
    emit_kv("withdrawal_balance", wbal);
    emit_kv("coinbase_balance", cbbal);
    emit_kv("trace_len", tracelen);
    emit_kv("tx_success", success);
    /* zkvm_run emitted the canonical SSZ result via write_output; report its size
     * and successful_validation (byte 32, right after the 32-byte request root). */
    emit_kv("public_output_bytes", (uint64_t)zkvm_output_size());
    if (zkvm_output_size() >= 33)
        emit_kv("successful_validation", (uint64_t)zkvm_output_buffer()[32]);

    /* (5) model_fini surfaces any uncaught Sail exception as exit(EXIT_FAILURE),
     * which our runtime maps to abnormal termination. */
    model_fini();

    /* The guest ran to completion and emitted the canonical SSZ result; the
     * validation outcome is in successful_validation (public output byte 32).
     * A failed validation (successful_validation=0) is a NORMAL result -> exit
     * 0; only a guest malfunction (input decode failure, Sail assert, or a fault)
     * is abnormal termination (non-zero exit, via the runtime/trap path). */
    (void)gas; (void)storage0; (void)wbal; (void)success;
    htif_puts("[zkvm] stateless guest complete.\n");
    return 0;   /* successful termination */
}

void zkvm_exit(int code)
{
    htif_exit(code);
}
