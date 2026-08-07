#ifndef GUEST_WORKSPACE_H
#define GUEST_WORKSPACE_H

#include <stddef.h>
#include <stdint.h>

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
GUEST_NORETURN void guest_panic(const char *file, const char *function, unsigned line);

/* Finish a successfully reported guest run without returning through the
 * partially evaluated generated model. */
GUEST_NORETURN void guest_finish(void);

#define GUEST_ABORT() guest_panic(__FILE__, __func__, __LINE__)

/*
 * Resource profile for the allocation-free optimized guest.
 *
 * Exact input-derived regions are claimed only after the preceding region is
 * sealed: witness rows, witness digest buckets, BAL account/storage rows,
 * expected BAL events, MPT update rows, and receipt records. The constants
 * below are reserved extents only for protocol maxima or genuinely dynamic
 * execution state whose final size is not known before execution. Every owner
 * checks its live length before writing and fails closed when the workspace is
 * exhausted. The backing storage is never heap-allocated, resized, or freed.
 *
 * Sixteen MiB of EVM memory is above the expansion affordable by the protocol
 * transaction gas limit. The other byte regions are deliberately separate so
 * a large encoding cannot consume memory owned by execution or guest output.
 */
enum {
  GUEST_WORKSPACE_BYTES = 256U * 1024U * 1024U,
  GUEST_EVM_MEMORY_BYTES = 16U * 1024U * 1024U,
  GUEST_SCRATCH_BYTES = 16U * 1024U * 1024U,
  GUEST_OUTPUT_BYTES = 1U * 1024U * 1024U,
  GUEST_ACCELERATOR_INPUT_BYTES = 2U * 1024U * 1024U,
  GUEST_ACCELERATOR_WORK_BYTES = 2U * 1024U * 1024U,
  GUEST_TRANSIENT_STORAGE_ENTRIES = 65536U,
  /* BLOCKHASH window: index d holds the ancestor at distance d + 1 and the
   * Sail ancestor_index type is range(0, 255). */
  GUEST_ANCESTOR_HASHES = 256U,
  /* Suspended continuations: one per interpreter call frame at the EVM
   * call-depth limit. */
  GUEST_SUSPENDED_FRAMES = 1024U,
  /* Operand-stack frames: call-depth limit 1024 + transaction frame + slack. */
  GUEST_OPERAND_FRAMES = 1100U,
  /* Words per operand frame: EVM stack limit 1024 + slack. */
  GUEST_OPERAND_FRAME_WORDS = 1032U,
  GUEST_CODE_DB_ENTRIES = 32768U,
  GUEST_CODE_INDEX_BUCKETS = 2U * GUEST_CODE_DB_ENTRIES,
  GUEST_CODE_BYTES = 16U * 1024U * 1024U,
  /* One byte per code position keeps JUMPDEST construction and lookup direct.
   * The code and analysis arenas have independent cursors but identical
   * worst-case live extents. */
  GUEST_JUMPDEST_BYTES = GUEST_CODE_BYTES,
  /* Amsterdam charges 7,816 execution gas per authorization.  Its 2^24
   * regular-transaction gas ceiling therefore admits at most 2,146 tuples.
   * The authorization tracker uses power-of-two open addressing below 50%
   * load, so the first sufficient workspace is 8,192 buckets. */
  GUEST_PREPARED_AUTHORIZATIONS = (1U << 24) / 7816U,
  GUEST_AUTHORIZATION_ENTRIES = 8192U,
  GUEST_LOG_RECORDS = 65536U,
  GUEST_LOG_TOPICS = 4U * GUEST_LOG_RECORDS,
  GUEST_LOG_DATA_BYTES = 16U * 1024U * 1024U,
  GUEST_STATE_JOURNAL_ENTRIES = 262144U,
  GUEST_RECEIPT_BYTES = 8U * 1024U * 1024U,
  GUEST_BAL_STORAGE_CHANGES = 65536U,
  GUEST_BAL_BALANCE_CHANGES = 32768U,
  GUEST_BAL_NONCE_CHANGES = 32768U,
  GUEST_BAL_CODE_CHANGES = 32768U,
};

/*
 * Claims stable storage from the optimized guest's pre-provisioned workspace.
 * This is initialization-time address assignment, not dynamic allocation:
 * claims are never freed or resized and the backing region is distinct from
 * the Sail runtime heap. The platform supplies the complete backing region
 * zero-filled at process/guest startup. Claims preserve those contents;
 * production block setup therefore only assigns addresses and relies on the
 * platform's clean-start contract. Dense rows must initialize every field
 * before it is read. Capacity exhaustion is fatal.
 */
void *workspace_claim(size_t count, size_t element_size, size_t alignment);
void workspace_init(void);
void workspace_begin_run(void);
void workspace_begin_witness_nodes(void);
void workspace_seal_witness_nodes(void);
void workspace_prepare_execution(uint32_t account_count, uint32_t storage_count,
                                 uint32_t expected_bal_entries, uint32_t transaction_count);

/* Workspace-owned modules bind their stable backing regions at the lifecycle
 * boundaries above. These are initialization hooks, not part of the Sail ABI. */
void accelerators_workspace_bind(void);
void code_workspace_bind(void);
void frame_stack_workspace_bind(void);
void memory_workspace_bind(void);
void output_workspace_bind(void);
void prepared_authorizations_workspace_bind(void);
void scratch_workspace_bind(void);
void stack_workspace_bind(void);
void state_runtime_workspace_bind(void);
void transient_workspace_bind(uint32_t capacity);
void mpt_workspace_bind(uint32_t account_count, uint32_t storage_count);
void bal_input_workspace_bind(uint32_t expected_count);

#ifdef EVMSAIL_NATIVE_TEST
/* Native-test reset primitives. Every nonempty workspace claim is recorded as
 * (first row, capacity, row size); adjacent claims of the same row type are
 * coalesced. The test driver clears the prior run before rebinding a new one,
 * reproducing the production clean-start contract without putting reset work
 * in block execution. */
void workspace_debug_zero_table(void *first_entry, size_t capacity, size_t element_size);
void workspace_debug_reset_allocations(void);
#endif

#define WORKSPACE_BIND(pointer, count)                                                             \
  do {                                                                                             \
    (pointer) = (__typeof__(pointer))workspace_claim((count), sizeof *(pointer),                   \
                                                     _Alignof(__typeof__(*(pointer))));            \
  } while (0)

#endif
