/* ======================================================================== */
/* PRIMITIVES: PROTOCOL FORKS                                               */
/*                                                                          */
/* The protocol forks the model executes, oldest to newest -- declaration   */
/* order IS the activation order, and every fork-gated rule compares on it  */
/* (fork_ge / fork_lt). Pure data -- no registers, no externs.              */
/* ======================================================================== */

enum Fork = {
    Frontier,
    Homestead,
    Byzantium,
    Constantinople,
    Istanbul,
    Berlin, /* EIP-2929/2930 access lists + warm/cold access costs        */
    London, /* EIP-1559 fee market + EIP-3529 refund reduction            */
    Paris,
    Shanghai, /* EIP-3651 warm coinbase, EIP-3855 PUSH0, EIP-3860 initcode  */
    Cancun, /* EIP-1153/4844; precompiles 0x01-0x0a                       */
    Prague, /* EIP-7623 calldata floor; +BLS 0x0b-0x11                    */
    Osaka, /* EIP-7883 modexp gas, EIP-7825 cap; +0x100                  */
    Amsterdam, /* EIP-7954 code/initcode size bump (32768/65536)             */
}

/* activation index = declaration order */
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

/* fork a is at least / strictly before fork b; overloaded onto the
 * comparison operators so fork gates read `k_fork >= Berlin` */
function fork_gteq(a : Fork, b : Fork) -> bool = fork_index(b) <= fork_index(a)

function fork_lt(a : Fork, b : Fork) -> bool = fork_index(a) < fork_index(b)

overload operator >= = { fork_gteq }

overload operator < = { fork_lt }

/* Deserialize an SSZ ProtocolFork index (execution-specs tuple(ProtocolFork)
 * order: ... Shanghai=14 Cancun=15 Prague=16 Osaka=17 BPO1=18 BPO2=19
 * Amsterdam=20) into the Fork whose EXECUTION RULES apply: blob-parameter-only
 * forks collapse to their base, unknown future indices to the newest. The raw
 * index is preserved separately (ChainConfig.fork_index) for exact checks. */
function fork_of_protocol_index(idx : protocol_fork_index) -> Fork =
    if 20 <= idx then {
        Amsterdam
    } else {
        if 17 <= idx then {
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
                            if (10 <= idx) & (idx <= 12) then {
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
