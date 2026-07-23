/*md
# Instrumentation scopes

Named stages of the stateless validation pipeline, used to attribute
md
# Instrumentation scopes

Named stages of the stateless validation pipeline, used to attribute
cycle counts when profiling the model.

!!! note "Implementation"
    Instrumentation only: the scope markers carry no protocol meaning and
    do not affect execution.

| Scope | Stage |
| ----- | ----- |
| `SCOPE_DECODE_INPUT` | Input decoding |
| `SCOPE_INDEX_WITNESS` | Witness indexing |
| `SCOPE_VALIDATE_PAYLOAD` | Payload commitments |
| `SCOPE_EXECUTE_BLOCK` | Block execution |
| `SCOPE_VALIDATE_RESULT` | Result validation |
| `SCOPE_COMPUTE_OUTPUT_ROOT` | Output root |
| `SCOPE_SERIALIZE_OUTPUT` | Output serialization |


! The pipeline stages, as scope identifiers.
let SCOPE_STATELESS_VALIDATION : bits(8) = 0x00
let SCOPE_DECODE_INPUT : bits(8) = 0x01
let SCOPE_INDEX_WITNESS : bits(8) = 0x02
let SCOPE_VALIDATE_PAYLOAD : bits(8) = 0x03
let SCOPE_EXECUTE_BLOCK : bits(8) = 0x04
let SCOPE_VALIDATE_RESULT : bits(8) = 0x05
let SCOPE_COMPUTE_OUTPUT_ROOT : bits(8) = 0x06
let SCOPE_SERIALIZE_OUTPUT : bits(8) = 0x07

!!! note "Implementation"
    Instrumentation only: the scope markers carry no protocol meaning and
    do not affect execution.

| Scope | Stage |
| ----- | ----- |
| `SCOPE_DECODE_INPUT` | Input decoding |
| `SCOPE_INDEX_WITNESS` | Witness indexing |
| `SCOPE_VALIDATE_PAYLOAD` | Payload commitments |
| `SCOPE_EXECUTE_BLOCK` | Block execution |
| `SCOPE_VALIDATE_RESULT` | Result validation |
| `SCOPE_COMPUTE_OUTPUT_ROOT` | Output root |
| `SCOPE_SERIALIZE_OUTPUT` | Output serialization |
*/

/*! The pipeline stages, as scope identifiers. */
let SCOPE_STATELESS_VALIDATION : bits(8) = 0x00
let SCOPE_DECODE_INPUT : bits(8) = 0x01
let SCOPE_INDEX_WITNESS : bits(8) = 0x02
let SCOPE_VALIDATE_PAYLOAD : bits(8) = 0x03
let SCOPE_EXECUTE_BLOCK : bits(8) = 0x04
let SCOPE_VALIDATE_RESULT : bits(8) = 0x05
let SCOPE_COMPUTE_OUTPUT_ROOT : bits(8) = 0x06
let SCOPE_SERIALIZE_OUTPUT : bits(8) = 0x07

/*! Profiling-only subdivisions used to attribute the dominant pipeline
    stages without changing the unprofiled model. */
let SCOPE_BLOCK_START : bits(8) = 0x08
let SCOPE_BLOCK_TRANSACTIONS : bits(8) = 0x09
let SCOPE_TX_DECODE : bits(8) = 0x0A
let SCOPE_TX_RESET : bits(8) = 0x0B
let SCOPE_TX_VALIDATE : bits(8) = 0x0C
let SCOPE_TX_UPFRONT : bits(8) = 0x0D
let SCOPE_TX_FRAME : bits(8) = 0x0E
let SCOPE_TX_SETTLE : bits(8) = 0x0F
let SCOPE_RECEIPTS_ROOT : bits(8) = 0x10
let SCOPE_BLOCK_END_STATE : bits(8) = 0x11
let SCOPE_BLOCK_END_REQUESTS : bits(8) = 0x12
let SCOPE_STATE_ROOT : bits(8) = 0x13
let SCOPE_BLOCK_ACCESS_LIST : bits(8) = 0x14
let SCOPE_HTR_EXECUTION_PAYLOAD : bits(8) = 0x15
let SCOPE_HTR_TRANSACTIONS : bits(8) = 0x16
let SCOPE_HTR_WITHDRAWALS : bits(8) = 0x17
let SCOPE_HTR_VERSIONED_HASHES : bits(8) = 0x18
let SCOPE_HTR_EXECUTION_REQUESTS : bits(8) = 0x19
let SCOPE_HTR_BYTES_ROOT : bits(8) = 0x1A
let SCOPE_HTR_MERKLE_PADDING : bits(8) = 0x1B
