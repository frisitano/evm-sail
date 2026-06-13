/* Validate the Sail Keccak-256 against known vectors. */

$include "../evm/evm.sail"
$include "evm_primitives.sail"
$include <string.sail>

function check(name : string, got : bits(256), want : string) -> unit = {
    let g = hex_str(unsigned(got));
    let ok = concat_str("0x", want) == g;
    print_endline(concat_str(name, concat_str(g, if ok then "  OK" else concat_str("  WANT ", want))));
    assert(ok, "keccak vector mismatch")
}

function main() -> unit = {
    check("keccak('')  = ", keccak256([||]), "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470");
    check(
        "keccak(abc) = ",
        keccak256([|0x61, 0x62, 0x63|]),
        "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45",
    );
    assert(
        create_address(0x6ac7ea33f8831ea9dcc53393aaa88b25a785dbf0, 0) == 0xcd234a471b72ba2f1ccf0a70fcaba648a5eecd8d,
        "create_address nonce 0"
    );
    assert(
        create_address(0x6ac7ea33f8831ea9dcc53393aaa88b25a785dbf0, 1) == 0x343c43a37d37dff08ae8c4a11544c718abb4fcf8,
        "create_address nonce 1"
    );
    print_endline("create_address vectors OK")
}
