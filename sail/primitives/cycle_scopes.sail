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
