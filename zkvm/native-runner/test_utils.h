/* Test/harness-only surface for the evm-sail stateless guest.
 *
 * This header + test_utils.c are linked ONLY into the harness shared library
 * (build_lib.sh -> libevmsail_guest.dylib), never into the production guest
 * binary. They own the "reuse layer" reset concern that we deliberately kept
 * OUT of the block model (sail/main.sail no longer calls k_world_reset): a
 * warm process can run many fixtures back-to-back by fully wiping guest state
 * between runs.
 *
 * A Python ctypes harness drives this: evmsail_lib_init() once, then per
 * fixture either fork() a child that calls evmsail_run_once() (fresh by
 * construction, no reset), or -- in a persistent worker -- evmsail_test_reset()
 * followed by evmsail_run_once().
 */
#ifndef EVMSAIL_TEST_UTILS_H
#define EVMSAIL_TEST_UTILS_H

/* model_init / model_fini around the whole session (once per process). */
void evmsail_lib_init(void);
void evmsail_lib_fini(void);

/* FULL wipe back to the post-model_init "fresh block" state: world overlay +
 * warm/journal/logs/refund/selfdestruct/created/bal/transient (k_world_reset),
 * the hash-keyed witness node DB (nodedb_reset), the k_witness_bad register
 * (set true on a deficient witness and NEVER self-reset -- the one leak an
 * FFI-only reset would miss), and the buffered output. Call between fixtures
 * ONLY in a reused (warm-worker) process; a forked child does not need it. */
void evmsail_test_reset(void);

/* The world wipe alone (no output-buffer reset). Calls the FFI resets directly;
 * needs no Sail-level k_world_reset. */
void evmsail_clear_memory(void);

/* Run the guest once over `in`[0..n) on a large-stack thread (the guest needs
 * a big stack for deep SSZ-list recursion). Resets the output buffers first,
 * so it is correct in BOTH the fork and warm-worker modes. On return, *out
 * points into the static output buffer (valid until the next run); the return
 * value is the output length in bytes. */
unsigned long evmsail_run_once(const unsigned char *in, unsigned long n,
                               const unsigned char **out);

/* Post-run analysis snapshot of the live FFI state (materialized accounts + storage,
 * stack, memory frame depth) as a self-describing big-endian blob; *out points
 * into a static buffer (valid until the next dump). See test_utils.c for the
 * layout; dump_state.py decodes it into Python types. */
unsigned long evmsail_dump_snapshot(const unsigned char **out);

#endif /* EVMSAIL_TEST_UTILS_H */
