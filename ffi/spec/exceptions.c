#include "exceptions.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

static bool decoded_input_present;
static bool writing_error;
static struct zStatelessInputRef decoded_input;

void fatal_error_reset(void)
{
  decoded_input_present = false;
  writing_error = false;
}

unit fatal_error_set_input(struct zStatelessInputRef input_ref)
{
  decoded_input = input_ref;
  decoded_input_present = true;
  return UNIT;
}

_Noreturn void fatal_error(enum zFatalError reason)
{
  (void)reason;
  if (writing_error) {
    abort();
  }
  writing_error = true;

  if (decoded_input_present) {
    (void)zwrite_validation_result(decoded_input, false);
  } else {
    (void)zwrite_invalid_result(UNIT);
  }
  pthread_exit(NULL);
}
