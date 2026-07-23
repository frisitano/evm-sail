
/*md
# System addresses and topics

The protocol-defined system actors: the pseudo-caller of block-level
system calls, the system predeploys they target, and the EIP-7708
transfer-log identity. Pure data — no registers, no externs.

| Name | Address / Topic | Source |
| ---- | --------------- | ------ |
| `SYSTEM_ADDRESS` | `0xfffe…fffe` | EIP-4788/2935/7002/7251 caller |
| `BEACON_ROOTS_ADDR` | `0x000f…ac02` | EIP-4788 |
| `HISTORY_STORAGE_ADDR` | `0x0000…2935` | EIP-2935 |
| `WITHDRAWAL_REQUEST_ADDR` | `0x0000…7002` | EIP-7002 |
| `CONSOLIDATION_REQUEST_ADDR` | `0x0000…7251` | EIP-7251 |
| `BUILDER_DEPOSIT_REQUEST_ADDR` | `0x0000…8282` | EIP-8282 |
| `BUILDER_EXIT_REQUEST_ADDR` | `0x0000…8282` | EIP-8282 |
| `DEPOSIT_CONTRACT_ADDR` | `0x0000…05fa` | EIP-6110 |
| `EIP7708_TRANSFER_TOPIC` | `keccak256("Transfer(…)")` | EIP-7708 |
*/
/*! The pseudo-caller of protocol system calls (EIP-4788, EIP-2935,
    EIP-7002, EIP-7251, EIP-8282). */
let SYSTEM_ADDRESS : address = word_to_address(
    U256(unsigned(0x000000000000000000000000fffffffffffffffffffffffffffffffffffffffe)),
)

/* system predeploys */
let BEACON_ROOTS_ADDR : address = word_to_address(
    U256(unsigned(0x000000000000000000000000000f3df6d732807ef1319fb7b8bb8522d0beac02)),
) /* EIP-4788 */
let HISTORY_STORAGE_ADDR : address = word_to_address(
    U256(unsigned(0x0000000000000000000000000000f90827f1c53a10cb7a02335b175320002935)),
) /* EIP-2935 */
let WITHDRAWAL_REQUEST_ADDR : address = word_to_address(
    U256(unsigned(0x00000000000000000000000000000961ef480eb55e80d19ad83579a64c007002)),
) /* EIP-7002 */
let CONSOLIDATION_REQUEST_ADDR : address = word_to_address(
    U256(unsigned(0x0000000000000000000000000000bbddc7ce488642fb579f8b00f3a590007251)),
) /* EIP-7251 */
let BUILDER_DEPOSIT_REQUEST_ADDR : address = word_to_address(
    U256(unsigned(0x0000000000000000000000000000bff46984e3725691fa540a8c7589300d8282)),
) /* EIP-8282 */
let BUILDER_EXIT_REQUEST_ADDR : address = word_to_address(
    U256(unsigned(0x000000000000000000000000000064d678505ad48f8ccb093bc65613800e8282)),
) /* EIP-8282 */

/* EIP-6110 deposit requests are reconstructed from DepositEvent logs. */
let DEPOSIT_CONTRACT_ADDR : address = word_to_address(
    U256(unsigned(0x00000000000000000000000000000000219ab540356cbb839cbe05303d7705fa)),
)
let DEPOSIT_EVENT_TOPIC : word = U256(unsigned(0x649bbc62d0e31342afea4e5cd82d4049e7e1ee912fc0889aa790803be39038c5))

/* EIP-7708: value transfers emit logs from the system log address */
let EIP7708_SYSTEM_ADDRESS : address = word_to_address(
    U256(unsigned(0x000000000000000000000000fffffffffffffffffffffffffffffffffffffffe)),
)
let EIP7708_TRANSFER_TOPIC : word = U256(unsigned(0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef))
let EIP7708_BURN_TOPIC : word = U256(unsigned(0xcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5))
