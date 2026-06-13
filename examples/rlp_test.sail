/* Validate RLP encoding against the standard vectors. */
$include "../evm/evm.sail"
$include "../evm/lib/rlp_decode.sail"
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

function check(name : string, got : list(byte), want : string) -> unit = {
  let g = bytes_hex(got);
  print_endline(concat_str(name, concat_str(g, if g == want then "  OK" else concat_str("  WANT ", want))));
  assert(g == want, "rlp vector mismatch")
}

function main() -> unit = {
  rlp_decode_selfcheck();
  print_endline("rlp_decode: nested list/field decode vectors  OK");
  check("rlp('')      = ", rlp_bytes([||]), "80");
  check("rlp(0x00)    = ", rlp_bytes([| 0x00 |]), "00");
  check("rlp(0x0f)    = ", rlp_bytes([| 0x0f |]), "0f");
  check("rlp('dog')   = ", rlp_bytes([| 0x64, 0x6f, 0x67 |]), "83646f67");
  check("rlp([])      = ", rlp_list([||]), "c0");
  check("rlp(int 0)   = ", rlp_int(0), "80");
  check("rlp(int 1024)= ", rlp_int(1024), "820400");
  /* ['cat','dog'] */
  let cat = rlp_bytes([| 0x63, 0x61, 0x74 |]);
  let dog = rlp_bytes([| 0x64, 0x6f, 0x67 |]);
  check("rlp(catdog)  = ", rlp_list(list_append(cat, dog)), "c88363617483646f67");
  print_endline("RLP vectors OK")
}
