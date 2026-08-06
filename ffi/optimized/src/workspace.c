#include "workspace.h"

#include <_abort.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef EVMSAIL_NATIVE_TEST
GUEST_NORETURN void native_panic(const char *file, const char *function, unsigned line);
GUEST_NORETURN void native_finish(void);
#endif

GUEST_NORETURN void guest_panic(const char *file, const char *function, unsigned line)
{
#ifdef EVMSAIL_NATIVE_TEST
  native_panic(file, function, line);
#else
  (void)file;
  (void)function;
  (void)line;
  abort();
#endif
}

GUEST_NORETURN void guest_finish(void)
{
#ifdef EVMSAIL_NATIVE_TEST
  native_finish();
#else
  exit(EXIT_SUCCESS);
#endif
}

#ifdef EVMSAIL_PLATFORM_ZISK
extern void *sys_alloc_aligned(size_t bytes, size_t alignment);
#elif defined(__riscv)
extern unsigned char __workspace_start[];
extern unsigned char __workspace_end[];
#else
static _Alignas(16) unsigned char native_workspace[GUEST_WORKSPACE_BYTES];
#endif

static unsigned char *cursor;
static unsigned char *limit;
static unsigned char *base;
static int initialized;

typedef enum {
  WORKSPACE_CLOSED,
  WORKSPACE_BOOTSTRAP,
  WORKSPACE_WITNESS_NODES,
  WORKSPACE_WITNESS_SUPPORT,
  WORKSPACE_EXECUTION,
} WorkspaceStage;

static WorkspaceStage stage;

#ifdef EVMSAIL_NATIVE_TEST
typedef struct {
  void *first_entry;
  size_t capacity;
  size_t element_size;
} WorkspaceDebugAllocation;

enum { WORKSPACE_DEBUG_ALLOCATION_CAPACITY = 256u };

static WorkspaceDebugAllocation workspace_debug_allocations[WORKSPACE_DEBUG_ALLOCATION_CAPACITY];
static size_t workspace_debug_allocation_count;

void workspace_debug_zero_table(void *first_entry, size_t capacity, size_t element_size)
{
  if (capacity == 0)
    return;
  if (first_entry == NULL || element_size == 0 || capacity > SIZE_MAX / element_size)
    GUEST_ABORT();
  memset(first_entry, 0, capacity * element_size);
}

static void workspace_debug_record_allocation(void *first_entry, size_t capacity,
                                              size_t element_size)
{
  if (capacity == 0)
    return;
  if (workspace_debug_allocation_count != 0) {
    WorkspaceDebugAllocation *previous =
        &workspace_debug_allocations[workspace_debug_allocation_count - 1u];
    unsigned char *previous_end =
        (unsigned char *)previous->first_entry + previous->capacity * previous->element_size;
    if (previous->element_size == element_size && previous_end == first_entry) {
      if (previous->capacity > SIZE_MAX - capacity)
        GUEST_ABORT();
      previous->capacity += capacity;
      return;
    }
  }
  if (workspace_debug_allocation_count == WORKSPACE_DEBUG_ALLOCATION_CAPACITY)
    GUEST_ABORT();
  workspace_debug_allocations[workspace_debug_allocation_count++] = (WorkspaceDebugAllocation){
      .first_entry = first_entry,
      .capacity = capacity,
      .element_size = element_size,
  };
}

void workspace_debug_reset_allocations(void)
{
  for (size_t i = 0; i < workspace_debug_allocation_count; ++i) {
    const WorkspaceDebugAllocation *allocation = &workspace_debug_allocations[i];
    workspace_debug_zero_table(allocation->first_entry, allocation->capacity,
                               allocation->element_size);
  }
  /* Claims are rebuilt from the workspace base for every fixture. Forget the
   * prior map after clearing it so the next run records exactly its own
   * input-dependent layout instead of accumulating stale/duplicate entries. */
  workspace_debug_allocation_count = 0;
}
#endif

void environment_workspace_bind(void);
void state_journal_workspace_bind(uint32_t capacity);
void state_logs_workspace_bind(uint32_t record_capacity, uint32_t topic_capacity,
                               uint32_t data_capacity);
void account_state_workspace_bind(uint32_t account_count, uint32_t storage_count);
void storage_state_workspace_bind(uint32_t storage_count);
void block_access_list_workspace_bind(uint32_t account_count, uint32_t storage_count);
void receipt_records_workspace_bind(uint32_t transaction_count);
void receipt_bytes_workspace_bind(size_t capacity);

/* Final lengths for these regions are not available before execution. Keep
 * their compile-time ceilings together, assign each an isolated workspace
 * interval, and pass the resulting capacity to the table that owns it. No
 * decoded input or protocol-profile budget participates in this layout. */
typedef struct {
  uint32_t journal_entries;
  uint32_t transient_entries;
  uint32_t log_records;
  uint32_t log_topics;
  uint32_t log_data_bytes;
  size_t receipt_bytes;
} DynamicRegionCapacities;

static const DynamicRegionCapacities dynamic_capacities = {
    .journal_entries = GUEST_STATE_JOURNAL_ENTRIES,
    .transient_entries = GUEST_TRANSIENT_STORAGE_ENTRIES,
    .log_records = GUEST_LOG_RECORDS,
    .log_topics = GUEST_LOG_TOPICS,
    .log_data_bytes = GUEST_LOG_DATA_BYTES,
    .receipt_bytes = GUEST_RECEIPT_BYTES,
};

_Static_assert((GUEST_TRANSIENT_STORAGE_ENTRIES & (GUEST_TRANSIENT_STORAGE_ENTRIES - 1U)) == 0,
               "transient-storage table size must be a power of two");

static void workspace_open(void)
{
  if (cursor != NULL) {
    return;
  }

#ifdef EVMSAIL_PLATFORM_ZISK
  cursor = sys_alloc_aligned(GUEST_WORKSPACE_BYTES, 16);
  if (cursor == NULL)
    GUEST_ABORT();
  limit = cursor + GUEST_WORKSPACE_BYTES;
#elif defined(__riscv)
  cursor = __workspace_start;
  limit = __workspace_end;
#else
  cursor = native_workspace;
  limit = native_workspace + sizeof native_workspace;
#endif

  base = cursor;
}

/* _Alignof yields a nonzero power of two. Counts may be input-derived, so the
 * multiplication is checked here before the aligned extent is calculated. */
void *workspace_claim(size_t count, size_t element_size, size_t alignment)
{
  workspace_open();
  if (element_size != 0 && count > SIZE_MAX / element_size) {
    GUEST_ABORT();
  }
  const size_t bytes = count * element_size;
  const uintptr_t current = (uintptr_t)cursor;
  const size_t padding = (size_t)(-current & (alignment - 1));
  if (padding > (size_t)(limit - cursor) || bytes > (size_t)(limit - cursor) - padding) {
    GUEST_ABORT();
  }

  cursor += padding;
  void *region = cursor;
  cursor += bytes;
#ifdef EVMSAIL_NATIVE_TEST
  workspace_debug_record_allocation(region, count, element_size);
#endif
  return region;
}

void workspace_init(void)
{
  if (initialized) {
    return;
  }
  initialized = 1;
  workspace_begin_run();
}

void workspace_begin_run(void)
{
  workspace_open();
  cursor = base;
  stage = WORKSPACE_BOOTSTRAP;
  accelerators_workspace_bind();
  output_workspace_bind();
  scratch_workspace_bind();
}

void workspace_begin_witness_nodes(void)
{
  if (stage != WORKSPACE_BOOTSTRAP) {
    GUEST_ABORT();
  }
  stage = WORKSPACE_WITNESS_NODES;
}

void workspace_seal_witness_nodes(void)
{
  if (stage != WORKSPACE_WITNESS_NODES) {
    GUEST_ABORT();
  }
  stage = WORKSPACE_WITNESS_SUPPORT;
  code_workspace_bind();
  environment_workspace_bind();
}

void workspace_prepare_execution(uint32_t account_count, uint32_t storage_count,
                                 uint32_t expected_bal_entries, uint32_t transaction_count)
{
  if (stage != WORKSPACE_WITNESS_SUPPORT) {
    GUEST_ABORT();
  }
  stage = WORKSPACE_EXECUTION;
  account_state_workspace_bind(account_count, storage_count);
  storage_state_workspace_bind(storage_count);
  block_access_list_workspace_bind(account_count, storage_count);
  bal_input_workspace_bind(expected_bal_entries);
  mpt_workspace_bind(account_count, storage_count);
  receipt_records_workspace_bind(transaction_count);

  /* Runtime regions begin only after all input-derived exact regions above
   * have been sealed. Each owner grows within its own [base, capacity) span. */
  stack_workspace_bind();
  frame_stack_workspace_bind();
  memory_workspace_bind();
  state_runtime_workspace_bind();
  state_journal_workspace_bind(dynamic_capacities.journal_entries);
  transient_workspace_bind(dynamic_capacities.transient_entries);
  state_logs_workspace_bind(dynamic_capacities.log_records, dynamic_capacities.log_topics,
                            dynamic_capacities.log_data_bytes);
  receipt_bytes_workspace_bind(dynamic_capacities.receipt_bytes);
}
