/* Minimal platform lifecycle for the generated Sail guest. Input and output
 * flow exclusively through the standard read_input/write_output interface. */

#include "sail.h"
#include "htif.h"
#ifdef EVMSAIL_OPTIMIZED_FFI
#include "workspace.h"
#endif

/* Sail-generated entry points (see build/zkvm_block.c). */
#ifdef EVMSAIL_OPTIMIZED_FFI
extern void evmsail_model_init(void);
#else
extern void model_init(void);
extern void model_fini(void);
#endif
extern unit zmain(unit);

int zkvm_start(void)
{
#ifdef EVMSAIL_OPTIMIZED_FFI
    workspace_init();
    evmsail_model_init();
#else
    model_init();
#endif
    zmain(UNIT);
#ifndef EVMSAIL_OPTIMIZED_FFI
    model_fini();
#endif
    return 0;
}

void zkvm_exit(int code)
{
    htif_exit(code);
}
