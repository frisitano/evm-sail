
/*md
# Protocol forks

The protocol forks the model executes, oldest to newest. Their singleton
integer values are the activation order, so ordinary integer comparisons
express every fork gate without an overloaded comparison or conversion.
Pure data — no registers, no externs.
*/
/*! Every supported protocol and schema fork, in activation order. This is the
    sole fork identity in the model: the decoded schema byte selects a
    `ProtocolProfile`, which stores one of these values. The bounded semantic
    type prevents values outside the supported fork sequence, while each named
    constant retains its precise singleton type for dependent profile typing. */
type Fork = range(0, 16)

/*! Singleton indices for the supported fork sequence. Keeping the numeric
    identities named lets dependent profile constraints refer to protocol
    forks without repeating their wire-order integers. */
type frontier_fork_value : Int = 0
/*! `Homestead`'s position in the activation order. */
type homestead_fork_value : Int = 1
/*! `Byzantium`'s position in the activation order. */
type byzantium_fork_value : Int = 2
/*! `Constantinople`'s position in the activation order. */
type constantinople_fork_value : Int = 3
/*! `Istanbul`'s position in the activation order. */
type istanbul_fork_value : Int = 4
/*! `Berlin`'s position in the activation order. */
type berlin_fork_value : Int = 5
/*! `London`'s position in the activation order. */
type london_fork_value : Int = 6
/*! `ArrowGlacier`'s position in the activation order. */
type arrow_glacier_fork_value : Int = 7
/*! `GrayGlacier`'s position in the activation order. */
type gray_glacier_fork_value : Int = 8
/*! `Paris`'s position in the activation order. */
type paris_fork_value : Int = 9
/*! `Shanghai`'s position in the activation order. */
type shanghai_fork_value : Int = 10
/*! First fork at which blob gas is active. Kept as a type-level singleton so
    profile equations and the runtime `Cancun` value share one boundary. */
type first_blob_fork_value : Int = 11
/*! `Prague`'s position in the activation order. */
type prague_fork_value : Int = 12
/*! `Osaka`'s position in the activation order. */
type osaka_fork_value : Int = 13
/*! `BPO1`'s position in the activation order. */
type bpo1_fork_value : Int = 14
/*! `BPO2`'s position in the activation order. */
type bpo2_fork_value : Int = 15
/*! `Amsterdam`'s position in the activation order. */
type amsterdam_fork_value : Int = 16

/*md
## Constants

The runtime fork values, one per supported fork. Each constant carries its
type-level index as a singleton type, so comparing two fork values compares
their activation order.

| Constants | Values | Character |
| --------- | ------ | --------- |
| `Frontier` … `Shanghai` | 0 – 10 | the pre-blob protocol and schema forks |
| `Cancun` | 11 | the first fork with an active blob schedule |
| `Prague`, `Osaka` | 12, 13 | blob-era execution forks |
| `BPO1`, `BPO2` | 14, 15 | blob-parameter-only schema forks |
| `Amsterdam` | 16 | the newest supported fork |
*/
/*! Launch protocol. */
let Frontier : int(frontier_fork_value) = sizeof(frontier_fork_value)
/*! EIP-2 create rules, EIP-7 DELEGATECALL. */
let Homestead : int(homestead_fork_value) = sizeof(homestead_fork_value)
/*! EIP-140 REVERT, EIP-211 returndata; precompiles 0x05-0x08. */
let Byzantium : int(byzantium_fork_value) = sizeof(byzantium_fork_value)
/*! EIP-145 shifts, EIP-1014 CREATE2, EIP-1052 EXTCODEHASH. */
let Constantinople : int(constantinople_fork_value) = sizeof(constantinople_fork_value)
/*! EIP-1344 CHAINID, EIP-2200 SSTORE metering. */
let Istanbul : int(istanbul_fork_value) = sizeof(istanbul_fork_value)
/*! EIP-2929/2930 access lists and warm/cold access costs. */
let Berlin : int(berlin_fork_value) = sizeof(berlin_fork_value)
/*! EIP-1559 fee market and EIP-3529 refund reduction. */
let London : int(london_fork_value) = sizeof(london_fork_value)
/*! Difficulty-bomb-only schema fork; execution rules remain London. */
let ArrowGlacier : int(arrow_glacier_fork_value) = sizeof(arrow_glacier_fork_value)
/*! Difficulty-bomb-only schema fork; execution rules remain London. */
let GrayGlacier : int(gray_glacier_fork_value) = sizeof(gray_glacier_fork_value)
/*! EIP-4399 PREVRANDAO replaces DIFFICULTY. */
let Paris : int(paris_fork_value) = sizeof(paris_fork_value)
/*! EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode. */
let Shanghai : int(shanghai_fork_value) = sizeof(shanghai_fork_value)
/*! EIP-1153/4844; precompiles 0x01-0x0a. */
let Cancun : int(first_blob_fork_value) = sizeof(first_blob_fork_value)
/*! EIP-7623 calldata floor; BLS precompiles 0x0b-0x11. */
let Prague : int(prague_fork_value) = sizeof(prague_fork_value)
/*! EIP-7883 modexp gas, EIP-7825 cap; precompile 0x100. */
let Osaka : int(osaka_fork_value) = sizeof(osaka_fork_value)
/*! First blob-parameter-only fork; execution rules remain Osaka. */
let BPO1 : int(bpo1_fork_value) = sizeof(bpo1_fork_value)
/*! Second blob-parameter-only fork; execution rules remain Osaka. */
let BPO2 : int(bpo2_fork_value) = sizeof(bpo2_fork_value)
/*! EIP-7954 code/initcode size bump (65536/131072). */
let Amsterdam : int(amsterdam_fork_value) = sizeof(amsterdam_fork_value)
