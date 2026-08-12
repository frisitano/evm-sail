/* Minimal platform lifecycle for the generated Sail guest. Input and output
 * flow exclusively through the standard read_input/write_output interface. */

#ifndef EVMSAIL_OPTIMIZED_FFI
#include "sail.h"
#endif
#include "htif.h"
#ifdef EVMSAIL_OPTIMIZED_FFI
#include "workspace.h"
#endif

/* Sail-generated entry points (see build/zkvm_block.c). */
#ifdef EVMSAIL_OPTIMIZED_FFI
extern void evmsail_model_init(void);
extern void zmain(void);
#else
extern void model_init(void);
extern void model_fini(void);
extern unit zmain(unit);
#endif

int zkvm_start(void)
{
#ifdef EVMSAIL_OPTIMIZED_FFI
    workspace_init();
    evmsail_model_init();
#else
    model_init();
#endif
#ifdef EVMSAIL_OPTIMIZED_FFI
    zmain();
#else
    zmain(UNIT);
#endif
#ifndef EVMSAIL_OPTIMIZED_FFI
    model_fini();
#endif
    return 0;
}

void zkvm_exit(int code)
{
    htif_exit(code);
}
