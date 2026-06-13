/* ere guest bridge: drive the existing Sail EVM stateless validator from ere's
 * zkvm-standards I/O. The model core is unchanged; only the I/O providers differ
 * from the spike build (there the runtime bakes the witness in and writes the
 * result over HTIF; here the ere/zkVM SDK supplies read_input/write_output).
 *
 * Build the model core (Sail-generated C + sailfix runtime + acc_shim + the
 * host_*.c backends + this file + el_input.c) with -DERE_GUEST and link against
 * the SDK runtime that exports read_input/write_output. Call evmsail_validate()
 * once from the guest main (src/main.rs). */
#include <stdint.h>
#include <stddef.h>

/* zkvm-standards guest I/O, provided by the ere/zkVM runtime */
extern void read_input(const uint8_t **buf_ptr, size_t *buf_size);

/* model entry points (Sail-generated + runtime) */
extern void model_init(void);
extern void model_fini(void);
extern void zzzkvm_run(int);                 /* Sail zkvm_run (mangled), arg = UNIT */

/* el_input.c (compiled -DERE_GUEST) */
extern void evmsail_set_input(const unsigned char *p, unsigned long n);
extern void evmsail_flush_output(void);

/* Validate one stateless block: read the witness from the host, run the model,
 * commit the canonical SSZ result. The validation outcome is public-output
 * byte 32 (successful_validation), exactly as on the spike target. */
void evmsail_validate(void)
{
    const uint8_t *in = 0; size_t len = 0;
    read_input(&in, &len);
    evmsail_set_input((const unsigned char *)in, (unsigned long)len);
    model_init();
    zzzkvm_run(0 /* UNIT */);
    evmsail_flush_output();
    model_fini();
}
