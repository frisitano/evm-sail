/* Minimal platform lifecycle for the generated Sail guest. Input and output
 * flow exclusively through the standard read_input/write_output interface. */

#include "sail.h"
#include "htif.h"

/* Sail-generated entry points (see build/zkvm_block.c). */
extern void model_init(void);
extern void model_fini(void);
extern unit zmain(unit);

int zkvm_start(void)
{
    model_init();
    zmain(UNIT);
    model_fini();
    return 0;
}

void zkvm_exit(int code)
{
    htif_exit(code);
}
