/* ======================================================================== */
/* PRIMITIVES: PROTOCOL SYSTEM ADDRESSES + TOPICS                           */
/*                                                                          */
/* The protocol-defined system actors: the pseudo-caller of block-level     */
/* system calls, the system predeploys they target, and the EIP-7708        */
/* transfer-log identity. Pure data -- no registers, no externs.            */
/* ======================================================================== */

/* the caller of protocol system calls (EIP-4788 / 2935 / 7002 / 7251) */
let SYSTEM_ADDRESS : address = 0xfffffffffffffffffffffffffffffffffffffffe

/* system predeploys */
let BEACON_ROOTS_ADDR : address = 0x000f3df6d732807ef1319fb7b8bb8522d0beac02 /* EIP-4788 */
let HISTORY_STORAGE_ADDR : address = 0x0000f90827f1c53a10cb7a02335b175320002935 /* EIP-2935 */
let WITHDRAWAL_REQUEST_ADDR : address = 0x00000961ef480eb55e80d19ad83579a64c007002 /* EIP-7002 */
let CONSOLIDATION_REQUEST_ADDR : address = 0x0000bbddc7ce488642fb579f8b00f3a590007251 /* EIP-7251 */

/* EIP-7708: value transfers emit logs from the system log address */
let EIP7708_SYSTEM_ADDRESS : address = 0xfffffffffffffffffffffffffffffffffffffffe
let EIP7708_TRANSFER_TOPIC : word = 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef
let EIP7708_BURN_TOPIC : word = 0xcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5
