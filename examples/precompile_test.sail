/* Validate the C-host precompiles (run_precompile) against known vectors. */
$include "../sail/el_ir.sail"
$include "evm_primitives.sail"
$include <string.sail>

function nib(n : int) -> string = match n {
  0=>"0",1=>"1",2=>"2",3=>"3",4=>"4",5=>"5",6=>"6",7=>"7",
  8=>"8",9=>"9",10=>"a",11=>"b",12=>"c",13=>"d",14=>"e",_=>"f"
}
function byte_hex(b : byte) -> string =
  concat_str(nib(ediv_int(unsigned(b), 16)), nib(emod_int(unsigned(b), 16)))
function bytes_hex(bs : list(byte)) -> string =
  match bs { [||] => "", b :: r => concat_str(byte_hex(b), bytes_hex(r)) }

function pc_check(name : string, num : int, input : list(byte), want : string) -> unit = {
  let (ok, out) = run_precompile(num, input);
  let g = bytes_hex(out);
  print_endline(concat_str(name, concat_str(g, if ok & (g == want) then "  OK" else concat_str("  WANT ", want))));
  assert(ok & (g == want), "precompile vector mismatch")
}

function main() -> unit = {
  /* sha256 (0x02) */
  pc_check("sha256('')    = ", 2, [||],
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  pc_check("sha256('abc') = ", 2, [| 0x61, 0x62, 0x63 |],
    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
  /* ripemd160 (0x03): 20-byte digest left-padded to 32 bytes */
  pc_check("ripemd160('') = ", 3, [||],
    "0000000000000000000000009c1185a5c5e9fc54612808977ee8f548b2258d31");
  pc_check("ripemd160(abc)= ", 3, [| 0x61, 0x62, 0x63 |],
    "0000000000000000000000008eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
  /* identity (0x04) */
  pc_check("identity(abc) = ", 4, [| 0x61, 0x62, 0x63 |], "616263");
  print_endline("precompiles: sha256 / ripemd160 / identity match known vectors  OK")
}
