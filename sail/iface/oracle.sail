/* ======================================================================== */
/* HOST INTERFACE CONTRACT: PRIVATE INPUT ORACLE AND OUTPUT TRACE           */
/*                                                                          */
/* Abstract `val` declarations for the stateless private-input oracle and   */
/* the guest public-output trace (the "Private input oracle" and "Output    */
/* trace" contract groups of proof/extern-boundary.md). All input reads     */
/* are consistent views over one bounded byte vector: the little-endian     */
/* and big-endian helpers are derived from the same bytes. Emitting a byte  */
/* appends exactly that byte to the guest output trace and preserves byte   */
/* well-formedness. Build implementations must refine these materialized    */
/* byte-list semantics.                                                     */
/* ======================================================================== */

/* One byte of the canonical SSZ public output, via write_output in the guest
 * runtime. Only guest entries call this; core specs leave it abstract. */
val emit_out : bits(8) -> unit

val ssz_src_len : unit -> bits(64)
val ssz_src_byte : int -> bits(8)

/* bulk readers: n (<=8) bytes from the input in LE / BE order. */
val ssz_src_le : (int, int) -> bits(64)
val ssz_src_be : (int, int) -> bits(64)
