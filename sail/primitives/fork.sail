
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

/*! Launch protocol. */
let Frontier : int(0) = 0
/*! EIP-2 create rules, EIP-7 DELEGATECALL. */
let Homestead : int(1) = 1
/*! EIP-140 REVERT, EIP-211 returndata; precompiles 0x05-0x08. */
let Byzantium : int(2) = 2
/*! EIP-145 shifts, EIP-1014 CREATE2, EIP-1052 EXTCODEHASH. */
let Constantinople : int(3) = 3
/*! EIP-1344 CHAINID, EIP-2200 SSTORE metering. */
let Istanbul : int(4) = 4
/*! EIP-2929/2930 access lists and warm/cold access costs. */
let Berlin : int(5) = 5
/*! EIP-1559 fee market and EIP-3529 refund reduction. */
let London : int(6) = 6
/*! Difficulty-bomb-only schema fork; execution rules remain London. */
let ArrowGlacier : int(7) = 7
/*! Difficulty-bomb-only schema fork; execution rules remain London. */
let GrayGlacier : int(8) = 8
/*! EIP-4399 PREVRANDAO replaces DIFFICULTY. */
let Paris : int(9) = 9
/*! EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode. */
let Shanghai : int(10) = 10
/*! EIP-1153/4844; precompiles 0x01-0x0a. */
let Cancun : int(11) = 11
/*! EIP-7623 calldata floor; BLS precompiles 0x0b-0x11. */
let Prague : int(12) = 12
/*! EIP-7883 modexp gas, EIP-7825 cap; precompile 0x100. */
let Osaka : int(13) = 13
/*! First blob-parameter-only fork; execution rules remain Osaka. */
let BPO1 : int(14) = 14
/*! Second blob-parameter-only fork; execution rules remain Osaka. */
let BPO2 : int(15) = 15
/*! EIP-7954 code/initcode size bump (65536/131072). */
let Amsterdam : int(16) = 16
