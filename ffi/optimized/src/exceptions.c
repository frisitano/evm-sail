#include "evmsail/exceptions.h"

#include "evmsail/spec/lib/ssz/stateless_input.h"
#include "evmsail/spec/exceptions.h"
#if defined(EVMSAIL_NATIVE_TEST) || defined(EVMSAIL_DEBUG)
#include "evmsail/spec/host/debug_enabled.h"
#else
#include "evmsail/spec/host/debug_disabled.h"
#endif
#include "evmsail/spec/executor/result.h"
#include "workspace.h"

#include <stdbool.h>
#include <stdint.h>

static bool decoded_input_present;
static bool writing_error;
static struct StatelessInputRef decoded_input;
static const uint8_t FATAL_ERROR_STAGE_UNKNOWN = UINT8_C(0);

void fatal_error_reset(void)
{
  decoded_input_present = false;
  writing_error = false;
}

void fatal_error_set_input(struct StatelessInputRef input_ref)
{
  decoded_input = input_ref;
  decoded_input_present = true;
}

_Noreturn void fatal_error(enum FatalError reason)
{
  /* A failure while constructing the failure output indicates a broken host
   * or resource invariant, not a second consensus verdict. */
  if (writing_error) {
    GUEST_ABORT();
  }
  writing_error = true;

  validation_debug_record(FATAL_ERROR_STAGE_UNKNOWN, reason);
  if (decoded_input_present) {
    write_validation_result(decoded_input, false);
  } else {
    write_invalid_result();
  }
  guest_finish();
}
