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

/* fork a is at least / strictly before fork b; overloaded onto the
 * comparison operators so fork gates read `k_fork >= Berlin` */
function fork_gteq(a : Fork, b : Fork) -> bool = fork_index(a) >= fork_index(b)

function fork_lt(a : Fork, b : Fork) -> bool = fork_index(a) < fork_index(b)

overload operator >= = { fork_gteq }

overload operator < = { fork_lt }
