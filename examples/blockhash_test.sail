/* Validate block hash against a real (Berlin) genesis header. */
$include "../evm/el_ir.sail"
$include "evm_primitives.sail"
$include <string.sail>

function main() -> unit = {
  let h = block_hash_legacy(
    /* parentHash */ 0x0000000000000000000000000000000000000000000000000000000000000000,
    /* uncleHash  */ 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347,
    /* coinbase   */ 0x0000000000000000000000000000000000000000,
    /* stateRoot  */ 0x626bd5d1816f5f7c443c37350c5ad14c5d54bca3001a51604c5f7480f00bbf72,
    /* txTrie     */ 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421,
    /* receiptTrie*/ 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421,
    /* bloom      */ zero_bytes(256),
    /* difficulty */ 131072,        /* 0x020000 */
    /* number     */ 0,
    /* gasLimit   */ 120000000,     /* 0x07270e00 */
    /* gasUsed    */ 0,
    /* timestamp  */ 0,
    /* extraData  */ [| 0x00 |],
    /* mixHash    */ 0x0000000000000000000000000000000000000000000000000000000000000000,
    /* nonce      */ zero_bytes(8));
  let g = hex_str(unsigned(h));
  let want = "0x12b0376ee0d1827e8ca60fdac5fe98b2fc83dc53068ba5adabafc3cf9e7044a4";
  print_endline(concat_str("block_hash = ", concat_str(g, if g == want then "  OK" else concat_str("  WANT ", want))));
  assert(g == want, "block hash mismatch")
}
