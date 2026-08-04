#include "workspace.h"

#include <stdint.h>
#include <stdlib.h>

#if defined(EVMSAIL_NATIVE_TEST)
GUEST_NORETURN void native_panic(const char *file, const char *function,
                                 unsigned line);
#endif

GUEST_NORETURN void guest_panic(const char *file, const char *function,
                                unsigned line) {
#if defined(EVMSAIL_NATIVE_TEST)
  native_panic(file, function, line);
#else
  (void)file;
  (void)function;
  (void)line;
  abort();
#endif
}

#if defined(EVMSAIL_PLATFORM_ZISK)
extern void *sys_alloc_aligned(size_t bytes, size_t alignment);
#elif defined(__riscv)
extern unsigned char __workspace_start[];
extern unsigned char __workspace_end[];
#else
static _Alignas(16) unsigned char native_workspace[GUEST_WORKSPACE_BYTES];
#endif

static unsigned char *cursor;
static unsigned char *limit;
static int initialized;

void accelerators_workspace_bind(void);
void code_workspace_bind(void);
void environment_workspace_bind(void);
void frame_stack_workspace_bind(void);
void memory_workspace_bind(void);
void mpt_node_table_workspace_bind(void);
void output_workspace_bind(void);
void scratch_workspace_bind(void);
void stack_workspace_bind(void);
void state_journal_workspace_bind(void);
void state_logs_workspace_bind(void);
void state_runtime_workspace_bind(void);
void state_store_workspace_bind(void);
void transient_workspace_bind(void);
void mpt_workspace_bind(void);
void bal_input_workspace_bind(void);
void receipts_workspace_bind(void);

static void workspace_open(void) {
  if (cursor != NULL) return;

#if defined(EVMSAIL_PLATFORM_ZISK)
  cursor = sys_alloc_aligned(GUEST_WORKSPACE_BYTES, 16);
  limit = cursor + GUEST_WORKSPACE_BYTES;
#elif defined(__riscv)
  cursor = __workspace_start;
  limit = __workspace_end;
#else
  cursor = native_workspace;
  limit = native_workspace + sizeof native_workspace;
#endif

  if (cursor == NULL) GUEST_ABORT();
}

void *workspace_claim(size_t count, size_t element_size, size_t alignment) {
  workspace_open();
  if (alignment == 0 || (alignment & (alignment - 1)) != 0)
    GUEST_ABORT();
  if (element_size != 0 && count > SIZE_MAX / element_size)
    GUEST_ABORT();

  const size_t bytes = count * element_size;
  const uintptr_t current = (uintptr_t)cursor;
  const uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
  const uintptr_t end = (uintptr_t)limit;
  if (aligned < current || aligned > end || bytes > (size_t)(end - aligned))
    GUEST_ABORT();

  cursor = (unsigned char *)aligned + bytes;
  return (void *)aligned;
}

void workspace_init(void) {
  if (initialized) return;
  initialized = 1;
  accelerators_workspace_bind();
  code_workspace_bind();
  environment_workspace_bind();
  frame_stack_workspace_bind();
  memory_workspace_bind();
  mpt_node_table_workspace_bind();
  output_workspace_bind();
  scratch_workspace_bind();
  stack_workspace_bind();
  state_journal_workspace_bind();
  state_logs_workspace_bind();
  state_runtime_workspace_bind();
  state_store_workspace_bind();
  bal_input_workspace_bind();
  transient_workspace_bind();
  mpt_workspace_bind();
  receipts_workspace_bind();
}
