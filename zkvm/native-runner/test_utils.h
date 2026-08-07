/* Test/harness-only surface for the evm-sail stateless guest.
 *
 * This header + test_utils.c are linked ONLY into the harness shared library
 * (build_lib.sh -> libevmsail_guest.dylib), never into the production guest
 * binary. They own the "reuse layer" reset concern that we deliberately kept
 * OUT of the block model (sail/main.sail no longer calls k_world_reset): a
 * warm process can run many fixtures back-to-back by fully wiping guest state
 * between runs.
 *
 * A Python ctypes harness drives this: guest_init() once, then per fixture
 * either fork() a child that calls guest_run() (fresh by construction, no
 * reset), or -- in a persistent worker -- guest_reset() followed by
 * guest_run().
 */
#ifndef EVMSAIL_TEST_UTILS_H
#define EVMSAIL_TEST_UTILS_H

#if defined(__GNUC__) || defined(__clang__)
#define GUEST_API __attribute__((visibility("default")))
#else
#define GUEST_API
#endif

/* model_init / model_fini around the whole session (once per process). */
GUEST_API void guest_init(void);
GUEST_API void guest_fini(void);

/* FULL wipe back to the post-model_init "fresh block" state. The optimized
 * backend clears the prior run's recorded workspace allocations here, then
 * resets owner cursors and buffered output; block execution itself assumes a
 * clean workspace. Call between fixtures ONLY in a reused (warm-worker)
 * process; a forked child does not need it. */
GUEST_API void guest_reset(void);

/* Run the guest once over `in`[0..n) on a large-stack thread (the guest needs
 * a big stack for deep SSZ-list recursion). Resets the output buffers first,
 * so it is correct in BOTH the fork and warm-worker modes. On return, *out
 * points into the static output buffer (valid until the next run); the return
 * value is the output length in bytes. */
GUEST_API unsigned long
guest_run(const unsigned char *in, unsigned long n,
          const unsigned char **out);

/* Empty after a successful run. The optimized native backend reports a
 * fail-closed host invariant here after unwinding to its test-only thread
 * boundary. The returned pointer remains valid until the next run. */
GUEST_API const char *guest_last_error(void);

/* On-demand post-run debug dump of the live FFI state and canonical main.sail
 * output as a self-describing big-endian blob; *out points into a static buffer
 * valid until the next dump. This function is native-test-only. */
GUEST_API unsigned long
guest_debug_dump(const unsigned char **out);

#endif /* EVMSAIL_TEST_UTILS_H */
