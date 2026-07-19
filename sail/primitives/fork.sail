
/*md
# Protocol forks

The protocol forks the model executes, oldest to newest. Declaration order
is the activation order, and every fork-gated rule in the specification
compares on it via [fork_gteq][] / [fork_lt][]. Pure data — no registers,
no externs.
*/
/*! The supported protocol forks, in activation order. Blob-parameter-only
    schema forks collapse to their base execution fork in
    [protocol_profile][]. */
enum Fork = {
    /* launch protocol */
    Frontier,
    /* EIP-2 create rules, EIP-7 DELEGATECALL */
    Homestead,
    /* EIP-140 REVERT, EIP-211 returndata; precompiles 0x05-0x08 */
    Byzantium,
    /* EIP-145 shifts, EIP-1014 CREATE2, EIP-1052 EXTCODEHASH */
    Constantinople,
    /* EIP-1344 CHAINID, EIP-2200 SSTORE metering */
    Istanbul,
    /* EIP-2929/2930 access lists + warm/cold access costs */
    Berlin,
    /* EIP-1559 fee market + EIP-3529 refund reduction */
    London,
    /* EIP-4399 PREVRANDAO replaces DIFFICULTY */
    Paris,
    /* EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode */
    Shanghai,
    /* EIP-1153/4844; precompiles 0x01-0x0a */
    Cancun,
    /* EIP-7623 calldata floor; +BLS 0x0b-0x11 */
    Prague,
    /* EIP-7883 modexp gas, EIP-7825 cap; +0x100 */
    Osaka,
    /* EIP-7954 code/initcode size bump (65536/131072) */
    Amsterdam,
}

/*! The activation index of a fork: its declaration position. */
function fork_index(f : Fork) -> protocol_fork_index = match f {
    Frontier => 0,
    Homestead => 1,
    Byzantium => 2,
    Constantinople => 3,
    Istanbul => 4,
    Berlin => 5,
    London => 6,
    Paris => 7,
    Shanghai => 8,
    Cancun => 9,
    Prague => 10,
    Osaka => 11,
    Amsterdam => 12,
}

/*! Fork `a` activates no earlier than fork `b`. Overloaded onto `>=` so
    fork gates read `k_fork >= Berlin`. */
function fork_gteq(a : Fork, b : Fork) -> bool = fork_index(b) <= fork_index(a)

/*! Fork `a` activates strictly before fork `b`; overloaded onto `<`. */
function fork_lt(a : Fork, b : Fork) -> bool = fork_index(a) < fork_index(b)

overload operator >= = { fork_gteq }

overload operator < = { fork_lt }
