/*md
# Protocol forks

The protocol forks the model executes, oldest to newest. Declaration order
is the activation order, and every fork-gated rule in the specification
compares on it via [fork_gteq][] / [fork_lt][]. Pure data — no registers,
no externs.
*/

/*! The supported protocol forks, in activation order. Blob-parameter-only
    forks are not distinct members; they collapse to their base fork (see
    [fork_of_protocol_index][]). */
enum Fork = {
    Frontier, /* launch protocol */
    Homestead, /* EIP-2 create rules, EIP-7 DELEGATECALL */
    Byzantium, /* EIP-140 REVERT, EIP-211 returndata; precompiles 0x05-0x08 */
    Constantinople, /* EIP-145 shifts, EIP-1014 CREATE2, EIP-1052 EXTCODEHASH */
    Istanbul, /* EIP-1344 CHAINID, EIP-2200 SSTORE metering */
    Berlin, /* EIP-2929/2930 access lists + warm/cold access costs        */
    London, /* EIP-1559 fee market + EIP-3529 refund reduction            */
    Paris, /* EIP-4399 PREVRANDAO replaces DIFFICULTY */
    Shanghai, /* EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode  */
    Cancun, /* EIP-1153/4844; precompiles 0x01-0x0a                       */
    Prague, /* EIP-7623 calldata floor; +BLS 0x0b-0x11                    */
    Osaka, /* EIP-7883 modexp gas, EIP-7825 cap; +0x100                  */
    Amsterdam, /* EIP-7954 code/initcode size bump (32768/65536)             */
}

/*! The activation index of a fork: its declaration position. */
function fork_index(f : Fork) -> int = match f {
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
function fork_gteq(a : Fork, b : Fork) -> bool = fork_index(a) >= fork_index(b)

/*! Fork `a` activates strictly before fork `b`; overloaded onto `<`. */
function fork_lt(a : Fork, b : Fork) -> bool = fork_index(a) < fork_index(b)

overload operator >= = { fork_gteq }

overload operator < = { fork_lt }

/*! Deserializes an SSZ `ProtocolFork` index (the consensus
    `ProtocolFork` order: … Shanghai=14, Cancun=15, Prague=16,
    Osaka=17, BPO1=18, BPO2=19, Amsterdam=20) into the [Fork][type-Fork]
    whose execution rules apply: blob-parameter-only forks collapse to
    their base fork, unknown future indices to the newest. The raw index
    is preserved separately (`ChainConfig.fork_index`) for exact checks. */
function fork_of_protocol_index(idx : int) -> Fork =
    if idx >= 20 then {
        Amsterdam
    } else {
        if idx >= 17 then {
            Osaka /* Osaka(17), BPO1(18), BPO2(19) */
        } else {
            if idx == 16 then {
                Prague
            } else {
                if idx == 15 then {
                    Cancun
                } else {
                    if idx == 14 then {
                        Shanghai
                    } else {
                        if idx == 13 then {
                            Paris
                        } else {
                            if (idx >= 10) & (idx <= 12) then {
                                London
                            } else {
                                if idx == 9 then {
                                    Berlin
                                } else {
                                    if idx == 7 then {
                                        Istanbul
                                    } else {
                                        if idx == 5 then {
                                            Byzantium
                                        } else {
                                            if idx == 1 then { Homestead } else { Frontier }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
