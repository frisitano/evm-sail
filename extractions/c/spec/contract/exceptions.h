#ifndef EVMSAIL_EXCEPTIONS_H
#define EVMSAIL_EXCEPTIONS_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

void fatal_error_reset(void);
unit fatal_error_set_input(struct zStatelessInputRef input_ref);
_Noreturn void fatal_error(enum zFatalError reason);

#endif
