/* C-include for the EEST runner build (the ssz_src input source + crypto/
 * memory shims live in their own .c files; this just pulls the decls). */
#ifndef EVM_SAIL_RUNNER_FFI_H
#define EVM_SAIL_RUNNER_FFI_H
#include "sail.h"
#include "../ffi/host_crypto.h"       /* direct host hash helpers */
#include "../ffi/precompiles.h"       /* staged precompile input/execution */
#include "../ffi/returndata.h"        /* C-backed RETURNDATA buffers */
#include "../ffi/memory.h"            /* C-backed EVM memory (O(1) read/write, per-frame) */
#include "../ffi/transient_storage.h"
#include "../ffi/stack.h"
#include "../ffi/code_db.h"
#include "../ffi/trie_node_db.h"
#include "../ffi/state_db.h"
/* SSZ byte-input source (definitions in runner_ffi.c: buffered stdin). */
void     ssz_src_len(sail_int rop, const unit u);
uint64_t ssz_src_byte(sail_int idx);
uint64_t ssz_src_le(sail_int off, sail_int n);
uint64_t ssz_src_be(sail_int off, sail_int n);
const uint8_t *evmsail_ssz_ptr(uint64_t off, uint64_t len);
#endif
