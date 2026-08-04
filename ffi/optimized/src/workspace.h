#ifndef GUEST_WORKSPACE_H
#define GUEST_WORKSPACE_H

#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#define GUEST_NORETURN __attribute__((noreturn))
#else
#define GUEST_NORETURN _Noreturn
#endif

/*
 * Fail-closed invariant boundary shared by the optimized backend.
 *
 * Production guests terminate immediately. The native test harness replaces
 * that terminal action with a same-thread jump to run_once(), allowing
 * Python to report the precise failing invariant without weakening zkVM
 * execution or attempting to continue a partially completed guest run.
 */
GUEST_NORETURN void guest_panic(const char *file, const char *function,
                                unsigned line);

#define GUEST_ABORT() guest_panic(__FILE__, __func__, __LINE__)

/*
 * Compile-time resource profile for the allocation-free optimized guest.
 *
 * These are implementation capacities, not prover hints. Every owner checks
 * its live length before writing and fails closed when the fixed workspace is
 * exhausted. Reset changes cursors only; backing storage is never allocated,
 * freed, or cleared wholesale between executions.
 *
 * Sixteen MiB of EVM memory is above the expansion affordable by the protocol
 * transaction gas limit. The other byte regions are deliberately separate so
 * a large encoding cannot consume memory owned by execution or guest output.
 */
enum {
  GUEST_WORKSPACE_BYTES = 256u * 1024u * 1024u,
  GUEST_EVM_MEMORY_BYTES = 16u * 1024u * 1024u,
  GUEST_SCRATCH_BYTES = 16u * 1024u * 1024u,
  GUEST_OUTPUT_BYTES = 1u * 1024u * 1024u,
  GUEST_ACCELERATOR_INPUT_BYTES = 2u * 1024u * 1024u,
  GUEST_ACCELERATOR_WORK_BYTES = 2u * 1024u * 1024u,
  GUEST_WITNESS_INDEX_ENTRIES = 32768u,
  GUEST_TRANSIENT_STORAGE_ENTRIES = 65536u,
  /* BLOCKHASH window: index d holds the ancestor at distance d + 1 and the
   * Sail ancestor_index type is range(0, 255). */
  GUEST_ANCESTOR_HASHES = 256u,
  /* Suspended continuations: one per interpreter call frame at the EVM
   * call-depth limit. */
  GUEST_SUSPENDED_FRAMES = 1024u,
  /* Operand-stack frames: call-depth limit 1024 + transaction frame + slack. */
  GUEST_OPERAND_FRAMES = 1100u,
  /* Words per operand frame: EVM stack limit 1024 + slack. */
  GUEST_OPERAND_FRAME_WORDS = 1032u,
  GUEST_CODE_DB_ENTRIES = 32768u,
  GUEST_CODE_INDEX_BUCKETS = 2u * GUEST_CODE_DB_ENTRIES,
  GUEST_CODE_BYTES = 16u * 1024u * 1024u,
  /* One byte per code position keeps JUMPDEST construction and lookup direct.
   * The code and analysis arenas have independent cursors but identical
   * worst-case live extents. */
  GUEST_JUMPDEST_BYTES = GUEST_CODE_BYTES,
  /* Amsterdam charges 7,816 execution gas per authorization.  Its 2^24
   * regular-transaction gas ceiling therefore admits at most 2,146 tuples.
   * The authorization tracker uses power-of-two open addressing below 50%
   * load, so the first sufficient workspace is 8,192 buckets. */
  GUEST_AUTHORIZATION_ENTRIES = 8192u,
  GUEST_LOG_RECORDS = 65536u,
  GUEST_LOG_TOPICS = 4u * GUEST_LOG_RECORDS,
  GUEST_LOG_DATA_BYTES = 16u * 1024u * 1024u,
  GUEST_STATE_JOURNAL_ENTRIES = 262144u,
  /* NodeIds are digest-bucket positions plus a dynamic tail for inline
   * nodes and storage-root identities, so the payload column spans the
   * full bucket range. */
  GUEST_MPT_WITNESS_NODES = GUEST_WITNESS_INDEX_ENTRIES + 8192u,
  GUEST_MPT_STORAGE_UPDATES = 32768u,
  GUEST_MPT_ACCOUNT_UPDATES = 8192u,
  GUEST_MPT_MERGE_ENTRIES = GUEST_MPT_STORAGE_UPDATES + 1u,
  GUEST_RECEIPT_RECORDS = 4096u,
  GUEST_RECEIPT_BYTES = 8u * 1024u * 1024u,
  GUEST_STATE_ACCOUNTS = 8192u,
  GUEST_ACCOUNT_INDEX_BUCKETS = 2u * GUEST_STATE_ACCOUNTS,
  GUEST_STATE_STORAGE = 32768u,
  GUEST_BAL_STORAGE_CHANGES = 65536u,
  GUEST_BAL_BALANCE_CHANGES = 32768u,
  GUEST_BAL_NONCE_CHANGES = 32768u,
  GUEST_BAL_CODE_CHANGES = 32768u,
  GUEST_BAL_EXPECTED_ENTRIES =
      2u * GUEST_STATE_ACCOUNTS + GUEST_STATE_STORAGE +
      GUEST_BAL_STORAGE_CHANGES + GUEST_BAL_BALANCE_CHANGES +
      GUEST_BAL_NONCE_CHANGES + GUEST_BAL_CODE_CHANGES,
};

/*
 * Claims stable storage from the optimized guest's pre-provisioned workspace.
 * This is initialization-time address assignment, not dynamic allocation:
 * claims are never freed or resized and the backing region is distinct from
 * the Sail runtime heap. The returned storage is zero-filled at process/guest
 * startup. Capacity exhaustion is fatal.
 */
void *workspace_claim(size_t count, size_t element_size, size_t alignment);
void workspace_init(void);

#define WORKSPACE_BIND(pointer, count)                                      \
  do {                                                                      \
    if ((pointer) == NULL) {                                                \
      (pointer) = workspace_claim((count), sizeof *(pointer),               \
                                  _Alignof(__typeof__(*(pointer))));        \
    }                                                                       \
  } while (0)

#endif
