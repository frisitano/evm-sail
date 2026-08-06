/* Native implementation of the same standard read_input/write_output ABI used
 * by the zkVM guest, plus the reusable test-process lifecycle. */
#include "evmsail/host/region_access.h"
#include "evmsail/host/frame_stack.h"
#include "evmsail/exceptions.h"
#include "evmsail/prelude.h"
#include "workspace.h"
#include "test/native.h"
#include "test_utils.h"
#include "zkvm_io.h"

#include <pthread.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The standard Sail RTS refers to these hooks, which a standalone generated
 * main normally owns.  Native model builds deliberately use --c-no-main and
 * provide their lifecycle here instead. */

/* --------------------------- standard I/O ------------------------------- */
static const uint8_t *g_in = NULL;
static size_t g_in_len = 0;

static void set_input(const uint8_t *p, size_t n)
{
  g_in = p;
  g_in_len = n;
  input_reset();
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
  if (size > available)
    size = available;
  if (size != 0)
    memcpy(g_out + g_out_len, output, size);
  g_out_len += size;
}

const uint8_t *native_output_data(size_t *size)
{
  *size = g_out_len;
  return g_out;
}

/* --------------------------- state resets ------------------------------- */
/* Reset surface -- all plain C FFI symbols, so the harness wipes state
 * WITHOUT any Sail-level reset function. This is the whole point: k_world_reset
 * is driver/test plumbing, not EVM semantics, so it does not belong in the
 * model. We call its constituent FFI resets directly instead. */
extern void acct_db_reset(void);           /* transaction + block account state */
extern void storage_db_reset(void);        /* transaction + block storage state */
extern void bal_reset(void);               /* EIP-7928 block-access-list accumulator */
extern void warm_reset(uint32_t);          /* EIP-2929 warm sets */
extern void transient_storage_reset(void); /* EIP-1153 transient storage */
extern void logs_reset(void);
extern void state_journal_reset(void);
extern void nodedb_reset(void);  /* hash-keyed witness node store */
extern void code_db_reset(void); /* content-addressed code store (missing-code tests) */

/* Reset the C-side logical cursors that replace k_world_reset. Every guest_reset
 * clears the tables recorded for the prior run before these owners reset their
 * metadata; the following workspace_begin_run starts a fresh allocation map. */
static void reset_world(void)
{
  /* block-level overlays -- the pieces per-tx k_tx_reset does NOT clear */
  acct_db_reset();
  storage_db_reset();
  bal_reset();
  /* ephemeral per-frame/per-tx state (also cleared by k_tx_reset, but a
     * zero-transaction block would otherwise inherit it) */
  warm_reset(1);
  transient_storage_reset();
  logs_reset();
  state_journal_reset();
  /* content-addressed witness stores -- cleared so a fixture that
     * under-specifies its witness FAILS (valid=false) instead of borrowing a
     * stale node/code that an EARLIER fixture registered. The node DB alone is
     * not enough: the "codes_missing_*" negative tests need the code DB too. */
  nodedb_reset();
  code_db_reset();
  frame_stack_reset();
}

void guest_reset(void)
{
  workspace_debug_reset_allocations();
  reset_world();
  workspace_begin_run();
  g_out_len = 0;
}

/* --------------------------- lifecycle + run ---------------------------- */
extern void evmsail_model_init(void);

void guest_init(void)
{
  workspace_init();
  evmsail_model_init();
}
void guest_fini(void) {}

/* Big stack: the guest recurses over multi-MB SSZ lists; the exe pins a 512 MB
 * main-thread stack (build.sh). In-process the ctypes caller's thread stack is
 * far smaller, so run the guest on a dedicated large-stack thread. */
#define GUEST_STACK_BYTES ((size_t)0x20000000)

static _Thread_local jmp_buf *panic_boundary;
static char last_error[1024];

static void record_thread_error(const char *operation, int error)
{
  (void)snprintf(last_error, sizeof last_error, "%s failed: %s", operation, strerror(error));
}

/* Native diagnostics may unwind only to the boundary installed on the same
 * guest thread. Production builds never link this function: guest_panic
 * terminates there instead. */
GUEST_NORETURN void native_panic(const char *file, const char *function, unsigned line)
{
  (void)snprintf(last_error, sizeof last_error, "optimized FFI invariant failed at %s:%u in %s()",
                 file ? file : "?", line, function ? function : "?");
  if (panic_boundary != NULL)
    longjmp(*panic_boundary, 1);
  abort();
}

GUEST_NORETURN void native_finish(void)
{
  if (panic_boundary != NULL)
    longjmp(*panic_boundary, 2);
  abort();
}

const char *guest_last_error(void)
{
  return last_error;
}

static void *run_thread(void *unused)
{
  (void)unused;
  jmp_buf boundary;
  panic_boundary = &boundary;
  if (setjmp(boundary) == 0)
    (void)zmain();
  panic_boundary = NULL;
  return NULL;
}

unsigned long guest_run(const unsigned char *in, unsigned long n, const unsigned char **out)
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
  if (error != 0)
    record_thread_error("pthread_join", error);

  *out = g_out;
  return (unsigned long)g_out_len;
}
