#include <lean/lean.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define GUEST_API __attribute__((visibility("default")))
#else
#define GUEST_API
#endif

extern lean_object *initialize_evm_x2dlean_x2drunner_Runner(uint8_t builtin);
extern lean_object *evmsail_lean_run_once(lean_object *input);
extern lean_object *evmsail_lean_debug_dump(lean_object *unit);
extern void lean_initialize_runtime_module(void);

static lean_object *g_output;
static lean_object *g_debug;
static int g_initialized;

static void replace_buffer(lean_object **destination, lean_object *value) {
  if (*destination) lean_dec_ref(*destination);
  *destination = value;
}

static lean_object *copy_input(const unsigned char *input,
                               unsigned long length) {
  lean_object *bytes = lean_alloc_sarray(1, (size_t)length, (size_t)length);
  if (length != 0) {
    memcpy(lean_sarray_cptr(bytes), input, (size_t)length);
  }
  return bytes;
}

static lean_object *take_io_value(lean_object *result) {
  if (lean_io_result_is_error(result)) {
    lean_io_result_show_error(result);
    lean_dec_ref(result);
    return NULL;
  }

  lean_object *value = lean_io_result_get_value(result);
  lean_inc_ref(value);
  lean_dec_ref(result);
  return value;
}

GUEST_API void guest_init(void) {
  if (g_initialized) return;

  lean_initialize_runtime_module();
  lean_set_panic_messages(false);
  lean_object *result =
      initialize_evm_x2dlean_x2drunner_Runner(1 /* builtin */);
  lean_set_panic_messages(true);
  lean_io_mark_end_initialization();
  if (lean_io_result_is_error(result)) {
    lean_io_result_show_error(result);
    lean_dec_ref(result);
    return;
  }
  lean_dec_ref(result);
  lean_init_task_manager();
  g_initialized = 1;
}

GUEST_API void guest_fini(void) {
  if (!g_initialized) return;
  replace_buffer(&g_output, NULL);
  replace_buffer(&g_debug, NULL);
  lean_finalize_task_manager();
  g_initialized = 0;
}

GUEST_API void guest_reset(void) {
  replace_buffer(&g_output, NULL);
  replace_buffer(&g_debug, NULL);
}

GUEST_API unsigned long
guest_run(const unsigned char *input, unsigned long length,
          const unsigned char **output) {
  if (!g_initialized) guest_init();

  lean_object *result = evmsail_lean_run_once(copy_input(input, length));
  replace_buffer(&g_output, take_io_value(result));
  if (!g_output) {
    *output = NULL;
    return 0;
  }

  *output = lean_sarray_cptr(g_output);
  return (unsigned long)lean_sarray_size(g_output);
}

GUEST_API unsigned long
guest_debug_dump(const unsigned char **output) {
  if (!g_initialized) guest_init();

  lean_object *result = evmsail_lean_debug_dump(lean_box(0));
  replace_buffer(&g_debug, take_io_value(result));
  if (!g_debug) {
    *output = NULL;
    return 0;
  }

  *output = lean_sarray_cptr(g_debug);
  return (unsigned long)lean_sarray_size(g_debug);
}
