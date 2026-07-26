import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Scratch

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open ByteRegionResult
open BlockError

/-! # RLP encoding and decoding

Recursive Length Prefix (Yellow Paper Appendix B), the canonical
serialization Ethereum uses for accounts, block headers, transactions,
receipts, and Merkle-Patricia trie nodes. RLP encodes a recursive
structure of byte strings and lists; it imposes no type semantics, only a
minimal length-prefixed framing.

A byte `< 0x80` is its own encoding; strings of length `n ≤ 55` use
`0x80 + n`; longer strings use `0xb7 + |len|` followed by the big-endian
length. Lists mirror this with the `0xc0`/`0xf7` evm_prefix bands over an
already-encoded payload. Integers encode as their minimal big-endian byte
string with no leading zeros. Decoding proceeds one level at a time; a
field reference retains the complete encoded item as a source slice and its
content length without copying; the content is always the corresponding
suffix.

Malformed RLP is a normal invalid-block outcome: the decoders throw
`InvalidBlock(RlpDecode)` at the point of detection, caught once at the
guest top level. -/

def RLP_SHORT_LENGTH_LIMIT : Nat := 55

def RLP_WORD_LENGTH_LIMIT : Nat := WORD_BYTE_LENGTH

def RLP_UINT64_LENGTH_LIMIT : Nat := EIGHT_BYTE_LENGTH

def RLP_ENCODED_WORD_LENGTH : Nat := 33

def RLP_ENCODED_ADDRESS_LENGTH : Nat := 21

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 33 -/
def rlp_scratch_small_length (value : Nat) : Nat :=
  value

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧ 0 ≤ right -/
def rlp_scratch_length_add (left : Nat) (right : Nat) : Nat :=
  (left + right)

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 31 -/
def word_byte_count (index : Nat) : Nat :=
  (index + 1)

/-- Materializes a word's minimal big-endian byte sequence and its length. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def minimal_word_bytes (w : Nat) : ((List (BitVec 8)) × Nat) := Id.run do
  let remaining : Nat := w
  let out : (List (BitVec 8)) := []
  let len : Nat := 0
  let (len, out, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (len, out, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, out, remaining) := loop_vars
      loop_vars :=
        let (len, out, remaining) : (Nat × (List (BitVec 8)) × Nat) :=
          if ((word_nonzero remaining) : Bool)
          then
            (let out : (List (BitVec 8)) := ((word_low_byte remaining) :: out)
            let remaining : Nat := (word_shift_right remaining 8)
            let len : Nat := (word_byte_count byte_index)
            (len, out, remaining))
          else (len, out, remaining)
        (len, out, remaining)
    (pure loop_vars) ) : Id (Nat × (List (BitVec 8)) × Nat) )
  (pure (out, len))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 255 -/
def rlp_nat_length_byte (value : Nat) : (BitVec 8) :=
  (get_slice_int 8 value 0)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_byte_length_byte (value : Nat) : SailM (BitVec 8) := do
  assert (value ≤b 255) "sail/lib/rlp/rlp.sail:98.23-98.24"
  let length := value
  (pure (get_slice_int 8 length 0))

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_length_word (value : Nat) : SailM Nat := do
  (word_of_nat_byte_count value)

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_string_prefix (len : Nat) (first : (BitVec 8)) : SailM ((List (BitVec 8)) × Nat) := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure ([], 0))
  else
    (do
      if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
      then (pure ([(0x80#8 + (rlp_nat_length_byte len))], 1))
      else
        (do
          let (length_bytes, length_len) ← do
            (pure (minimal_word_bytes (← (rlp_length_word len))))
          (pure (((0xB7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (1 + length_len)))))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_list_prefix (len : Nat) : SailM ((List (BitVec 8)) × Nat) := do
  if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure ([(0xC0#8 + (rlp_nat_length_byte len))], 1))
  else
    (do
      let (length_bytes, length_len) ← do (pure (minimal_word_bytes (← (rlp_length_word len))))
      (pure (((0xF7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (1 + length_len))))

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def rlp_minimal_word_len (w : Nat) : Nat :=
  (word_byte_length w)

/- Type quantifiers: value : Nat, (rlp_natural_increment_valid value) -/
def rlp_natural_increment (value : Nat) : Nat :=
  (value + 1)

/-- Computes a natural's minimal big-endian byte length recursively. -/
/- Type quantifiers: _reclimit : Nat, n : Nat, (rlp_natural_valid n), 0 ≤ _reclimit -/
def _rec_rlp_minimal_uint_len (n : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((n == 0) : Bool)
      then (pure 0)
      else (pure (rlp_natural_increment (← (_rec_rlp_minimal_uint_len (n / 256) _reclimit_pred)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Computes a natural's minimal big-endian byte length recursively. -/
/- Type quantifiers: n : Nat, (rlp_natural_valid n) -/
def rlp_minimal_uint_len (n : Nat) : SailM Nat := do
  let _measure := (n : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_minimal_uint_len n (_measure + 1))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_length_prefix_len (len : Nat) : SailM Nat := do
  if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure 1)
  else (pure (1 + (rlp_minimal_word_len (← (rlp_length_word len)))))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_string_size (len : Nat) (first : (BitVec 8)) : SailM Nat := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure 1)
  else (pure ((← (rlp_length_prefix_len len)) + len))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_bytes_size (data : (List (BitVec 8))) (len : Nat) : SailM Nat := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_string_size len first)

/- Type quantifiers: k_off : Nat, k_len : Nat, (source_valid_range k_off k_len) -/
def rlp_slice_size (data : (EvmByteSliceFields k_off k_len)) : SailM Nat := do
  let len := k_len
  let first ← do
    if ((len == 0) : Bool)
    then (pure 0x00#8)
    else (slice_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_string_size len first)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def rlp_uint_word_size (w : Nat) : Nat :=
  let len := (rlp_minimal_word_len w)
  let first :=
    if ((len == 1) : Bool)
    then (word_low_byte w)
    else 0x00#8
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then 1
  else (1 + len)

/- Type quantifiers: n : Nat, (rlp_natural_valid n) -/
def rlp_uint_nat_size (n : Nat) : SailM Nat := do
  if ((n <b (2 ^i 256)) : Bool)
  then (pure (rlp_uint_word_size (U256 n)))
  else
    (do
      let len ← do (rlp_minimal_uint_len n)
      if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
      then (pure (1 + len))
      else (pure ((1 + (← (rlp_minimal_uint_len len))) + len)))

/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_uint_u64_size (n : Nat) : Nat :=
  if ((n <b (2 ^i 7)) : Bool)
  then 1
  else
    (if ((n <b (2 ^i 8)) : Bool)
    then 2
    else
      (if ((n <b (2 ^i 16)) : Bool)
      then 3
      else
        (if ((n <b (2 ^i 24)) : Bool)
        then 4
        else
          (if ((n <b (2 ^i 32)) : Bool)
          then 5
          else
            (if ((n <b (2 ^i 40)) : Bool)
            then 6
            else
              (if ((n <b (2 ^i 48)) : Bool)
              then 7
              else
                (if ((n <b (2 ^i 56)) : Bool)
                then 8
                else 9)))))))

def rlp_word_size (_ : Unit) : Nat :=
  RLP_ENCODED_WORD_LENGTH

def rlp_addr_size (_ : Unit) : Nat :=
  RLP_ENCODED_ADDRESS_LENGTH

/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def rlp_list_size (content_len : Nat) : SailM Nat := do
  (pure ((← (rlp_length_prefix_len content_len)) + content_len))

/-- Adds the canonical list evm_prefix to a materializable RLP content length. -/
/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def rlp_scratch_list_size (content_len : Nat) : SailM Nat := do
  (pure (rlp_scratch_length_add content_len
      (rlp_scratch_small_length (← (rlp_length_prefix_len content_len)))))

/-- Returns the materializable RLP width of a source-backed byte slice. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def rlp_scratch_slice_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  (rlp_slice_size data)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def rlp_scratch_bytes_size (data : (List (BitVec 8))) (len : Nat) : SailM Nat := do
  (rlp_bytes_size data len)

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_write_string_prefix (len : Nat) (first : (BitVec 8)) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_string_prefix len first)
  (scratch_push_bytes encoded_prefix prefix_len)

/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def rlp_write_list_prefix (content_len : Nat) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_list_prefix content_len)
  (scratch_push_bytes encoded_prefix prefix_len)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def rlp_write_bytes (data : (List (BitVec 8))) (len : Nat) : SailM Unit := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_write_string_prefix len first)
  (scratch_push_bytes data len)

/-- Appends a source-backed byte slice as an RLP string. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def rlp_write_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  (rlp_write_string_prefix data.len
    (← do
      if ((data.len == 0) : Bool)
      then (pure 0x00#8)
      else (slice_byte ⟨_, ⟨_, data⟩⟩ 0)))
  (scratch_push_slice ⟨_, ⟨_, data⟩⟩)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def rlp_write_uint_word (w : Nat) : SailM Unit := do
  let (bytes, len) := (minimal_word_bytes w)
  (rlp_write_bytes bytes len)

/- Type quantifiers: n : Nat, (rlp_natural_valid n) -/
def rlp_write_uint_nat (n : Nat) : SailM Unit := do
  if ((n <b (2 ^i 256)) : Bool)
  then (rlp_write_uint_word (U256 n))
  else sailThrow ((InvalidBlock InvalidConfig))

/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_write_uint_u64 (n : Nat) : SailM Unit := do
  (rlp_write_uint_word (U256 n))

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def rlp_write_word (w : Nat) : SailM Unit := do
  (rlp_write_bytes (word_to_bytes32 w) WORD_BYTE_LENGTH)

def rlp_write_addr (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  (rlp_write_bytes (address_to_bytes a) ADDRESS_BYTE_LENGTH)

/-- Finishes a forward scratch encoding. -/
/- Type quantifiers: start : Nat, 0 ≤ start -/
def rlp_finish (start : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  (scratch_finish start)

/- Type quantifiers: _width : Nat, evm_prefix' : Nat, 1 ≤ _width ∧
  _width ≤ 8 ∧ 0 ≤ evm_prefix' ∧ evm_prefix' ≤ (2 ^ (8 * (_width - 1)) - 1) -/
def rlp_uint64_append (_width : Nat) (evm_prefix' : Nat) (next : (BitVec 8)) : Nat :=
  ((evm_prefix' *i 256) + (BitVec.toNatInt next))

/-- Decodes exactly `width` big-endian bytes into a bounded unsigned value. -/
/- Type quantifiers: _reclimit : Nat, content_dependentWitness1 : Nat, content_dependentWitness0 :
  Nat, width : Nat, 0 ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧
  0 ≤ content_dependentWitness1, 0 ≤ _reclimit -/
def _rec_rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (width : Nat) (_reclimit : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((width == 0) : Bool)
      then (pure 0)
      else
        (pure (rlp_uint64_append width
            (← (_rec_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ (width - 1) _reclimit_pred))
            (← (slice_byte ⟨_, ⟨_, content⟩⟩ (width - 1))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Decodes exactly `width` big-endian bytes into a bounded unsigned value. -/
/- Type quantifiers: content_dependentWitness1 : Nat, content_dependentWitness0 : Nat, width : Nat, 0
  ≤ width ∧ width ≤ 8, 0 ≤ content_dependentWitness0 ∧ 0 ≤ content_dependentWitness1 -/
def rlp_uint64_width (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (width : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  let _measure := (width : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width (_measure + 1))

/- Type quantifiers: source_dependentWitness1 : Nat, source_dependentWitness0 : Nat, start : Nat, (source_valid_length start), 0
  ≤ source_dependentWitness0 ∧ 0 ≤ source_dependentWitness1 -/
def rlp_bytes_equal_at (expected : (List (BitVec 8))) (source : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (start : Nat) : SailM Bool := do
  let source_dependentWitness0 := (source).1
  let source_dependentWitness1 := ((source).2).1
  let source := ((source).2).2
  let rest : (List (BitVec 8)) := expected
  let current : Nat := start
  let equal : Bool := true
  let (current, equal, rest) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (current, equal, rest)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (current, equal, rest) := loop_vars
      loop_vars ← do
        let (current, equal, rest) ← (( do
          match rest with
          | (b :: tail) =>
            (do
              let position := current
              let source_length := source.len
              let (current, equal) ← (( do
                if ((position <b source_length) : Bool)
                then
                  (do
                    let equal ←
                      if (equal : Bool)
                      then
                        (do
                          (pure ((← (slice_byte ⟨_, ⟨_, source⟩⟩ current)) == b)))
                      else (pure false)
                    let current : Nat := (position + 1)
                    (pure (current, equal)))
                else
                  (let equal : Bool := false
                  (pure (current, equal))) ) : SailM (Nat × Bool) )
              let rest : (List (BitVec 8)) := tail
              (pure (current, equal, rest)))
          | [] => (pure (current, equal, rest)) ) : SailM (Nat × Bool × (List (BitVec 8))) )
        (pure (current, equal, rest))
    (pure loop_vars) ) : SailM (Nat × Bool × (List (BitVec 8))) )
  match rest with
  | [] => (pure equal)
  | _ => (pure false)

/-- Decodes the first RLP header in an exact remaining source slice:
`(is_list, content_offset, content_length)`. -/
/- Type quantifiers: b_dependentWitness1 : Nat, b_dependentWitness0 : Nat, 0 ≤ b_dependentWitness0
  ∧ 0 ≤ b_dependentWitness1 -/
def rlp_ref_hdr (b : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Bool × Nat × Nat) := do
  let b_dependentWitness0 := (b).1
  let b_dependentWitness1 := ((b).2).1
  let b := ((b).2).2
  let source_len := b.len
  if ((source_len == 0) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else
    (do
      let h ← do (pure (BitVec.toNatInt (← (slice_byte ⟨_, ⟨_, b⟩⟩ 0))))
      if ((h <b 128) : Bool)
      then (pure (false, 0, 1))
      else
        (do
          if ((h <b 184) : Bool)
          then (pure (false, 1, (h - 128)))
          else
            (do
              if ((h <b 192) : Bool)
              then
                (do
                  let length_width : Nat := (h - 183)
                  if (((source_len - 1) <b length_width) : Bool)
                  then sailThrow ((InvalidBlock RlpDecode))
                  else
                    (do
                      let length_bytes := (sub_slice b 1 length_width)
                      (pure (false, (1 + length_width), (← (rlp_uint64_width
                            ⟨_, ⟨_, length_bytes⟩⟩ length_width))))))
              else
                (do
                  if ((h <b 248) : Bool)
                  then (pure (true, 1, (h - 192)))
                  else
                    (do
                      let length_width : Nat := (h - 247)
                      if (((source_len - 1) <b length_width) : Bool)
                      then sailThrow ((InvalidBlock RlpDecode))
                      else
                        (do
                          let length_bytes := (sub_slice b 1 length_width)
                          (pure (true, (1 + length_width), (← (rlp_uint64_width
                                ⟨_, ⟨_, length_bytes⟩⟩ length_width))))))))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_cursor (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (EvmByteSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len) := do
  if (f.is_list : Bool)
  then (pure (sub_slice f.source (k_source_len - k_content_len) k_content_len))
  else sailThrow ((InvalidBlock RlpDecode))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_cursor_pop (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM (Sigma fun
  (k_content_len : Nat) =>
  (Sigma fun (k_full_len : Nat) =>
  ((RlpFieldRef k_source_off k_full_len k_content_len) × (EvmByteSliceFields (k_source_off + k_full_len) (k_source_len - k_full_len))))) := do
  if _sailIf0 : ((k_source_len == 0) : Bool) = true
  then
    (do
      sailThrow ((InvalidBlock RlpDecode)))
  else
    (do
      let (is_list, content_off, content_len_value) ← do (rlp_ref_hdr ⟨_, ⟨_, cursor⟩⟩)
      let content_len := content_len_value
      if _sailIf1 : ((k_source_len <b content_off) : Bool) = true
      then
        (do
          sailThrow ((InvalidBlock RlpDecode)))
      else
        (do
          if _sailIf2 : (((k_source_len - content_off) <b content_len) : Bool) = true
          then
            (do
              sailThrow ((InvalidBlock RlpDecode)))
          else
            (do
              let full_len := (content_off + content_len)
              if _sailIf3 : (((0 <b full_len) && (full_len ≤b k_source_len)) : Bool) = true
              then
                (let field_source := (sub_slice cursor 0 full_len)
                let field : (RlpFieldRef k_source_off full_len content_len) :=
                  { source := field_source,
                    is_list := is_list,
                    content_len := content_len }
                let advanced := (sub_slice cursor full_len (k_source_len - full_len))
                (pure ((⟨_, ⟨_, (field, advanced)⟩⟩ : (Sigma fun (k_syn_content_len : Nat)
                  =>
                  (Sigma fun (k_syn_full_len : Nat) =>
                  ((RlpFieldRef k_source_off k_syn_full_len k_syn_content_len) × (EvmByteSliceFields (k_source_off + k_syn_full_len) (k_source_len - k_syn_full_len)))))) : (Sigma
                  fun (k_syn_content_len : Nat) =>
                  (Sigma fun (k_syn_full_len : Nat) =>
                  ((RlpFieldRef k_source_off k_syn_full_len k_syn_content_len) × (EvmByteSliceFields (k_source_off + k_syn_full_len) (k_source_len - k_syn_full_len))))))))
              else
                (do
                  sailThrow ((InvalidBlock RlpDecode))))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_cursor_expect_end (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM Unit := do
  if ((k_source_len == 0) : Bool)
  then (pure ())
  else sailThrow ((InvalidBlock RlpDecode))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def rlp_single_ref (item : (EvmByteSliceFields k_source_off k_source_len)) : SailM (Sigma fun
  (k_content_len : Nat) => (RlpFieldRef k_source_off k_source_len k_content_len)) := do
  let item_length := k_source_len
  if _sailIf0 : ((item_length == 0) : Bool) = true
  then
    (do
      sailThrow ((InvalidBlock RlpDecode)))
  else
    (do
      let (is_list, content_off, content_len_value) ← do (rlp_ref_hdr ⟨_, ⟨_, item⟩⟩)
      let content_len := content_len_value
      if _sailIf1 : ((content_off >b item_length) : Bool) = true
      then
        (do
          sailThrow ((InvalidBlock RlpDecode)))
      else
        (do
          if _sailIf2 : ((content_len != (item_length - content_off)) : Bool) = true
          then
            (do
              sailThrow ((InvalidBlock RlpDecode)))
          else
            (let field : (RlpFieldRef k_source_off k_source_len content_len) :=
              { source := item,
                is_list := is_list,
                content_len := content_len }
            (pure ((⟨_, field⟩ : (Sigma fun (k_syn_content_len : Nat) =>
              (RlpFieldRef k_source_off k_source_len k_syn_content_len))) : (Sigma fun
              (k_syn_content_len : Nat) => (RlpFieldRef k_source_off k_source_len k_syn_content_len)))))))

/-- A cursor over the children of a byte sequence that must be exactly
one RLP list (e.g. a trie node). -/
/- Type quantifiers: node_dependentWitness1 : Nat, node_dependentWitness0 : Nat, 0 ≤
  node_dependentWitness0 ∧ 0 ≤ node_dependentWitness1 -/
def rlp_node_cursor (node : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Sigma fun
  (k_source_off : Nat) =>
  (Sigma fun (k_source_len : Nat) => (EvmByteSliceFields k_source_off k_source_len))) := do
  let node_dependentWitness0 := (node).1
  let node_dependentWitness1 := ((node).2).1
  let node := ((node).2).2
  (do
    let dependentResult ← (do
        let dependentArg0 := (← (rlp_single_ref node))
        let publicResult ← (rlp_ref_cursor (dependentArg0).2)
        pure ((⟨_, publicResult⟩ : (Sigma fun (k_ex420115_ : Nat) =>
        (EvmByteSliceFields (node_dependentWitness0 + (node_dependentWitness1 - k_ex420115_)) k_ex420115_)))))
    pure ((⟨_, ⟨_, (dependentResult).2⟩⟩ : (Sigma fun (k_source_off : Nat) =>
    (Sigma fun (k_source_len : Nat) => (EvmByteSliceFields k_source_off k_source_len))))))

/-- The content span of a field. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_content (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
  ((⟨_, ⟨_, (sub_slice f.source (k_source_len - k_content_len) k_content_len)⟩⟩ : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))

/-- Whether a field uses the unique canonical RLP framing for its payload. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_framing_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  let n := k_content_len
  let payload_length := n
  let full_length := k_source_len
  let full_offset := 0
  let content_offset := (full_length - payload_length)
  let source := f.source
  let source_length := source.len
  if ((full_length == 0) : Bool)
  then (pure false)
  else
    (do
      if (f.is_list : Bool)
      then
        (do
          if ((n ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
          then
            if ((full_length == (payload_length + 1)) : Bool)
            then
              (do
                (pure ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xC0#8 + (rlp_nat_length_byte
                        n)))))
            else (pure false)
          else
            (do
              let (len_bytes, length_width) ← do
                (pure (minimal_word_bytes (← (rlp_length_word n))))
              if ((full_length == ((1 + length_width) + payload_length)) : Bool)
              then
                (do
                  if (((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xF7#8 + (rlp_nat_length_byte
                           length_width))) : Bool)
                  then
                    (do
                      (rlp_bytes_equal_at len_bytes ⟨_, ⟨_, f.source⟩⟩ (full_offset + 1)))
                  else (pure false))
              else (pure false)))
      else
        (do
          if ((payload_length == 0) : Bool)
          then
            if ((full_length == 1) : Bool)
            then
              (do
                (pure ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == 0x80#8)))
            else (pure false)
          else
            (do
              let first ← do (slice_byte ⟨_, ⟨_, f.source⟩⟩ content_offset)
              if (((payload_length == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
              then
                if ((full_length == 1) : Bool)
                then
                  (do
                    (pure ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == first)))
                else (pure false)
              else
                (do
                  if ((n ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
                  then
                    if ((full_length == (payload_length + 1)) : Bool)
                    then
                      (do
                        (pure ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0x80#8 + (rlp_nat_length_byte
                                n)))))
                    else (pure false)
                  else
                    (do
                      let (len_bytes, length_width) ← do
                        (pure (minimal_word_bytes (← (rlp_length_word n))))
                      if ((full_length == ((1 + length_width) + payload_length)) : Bool)
                      then
                        (do
                          if (((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ 0)) == (0xB7#8 + (rlp_nat_length_byte
                                   length_width))) : Bool)
                          then
                            (do
                              (rlp_bytes_equal_at len_bytes ⟨_, ⟨_, f.source⟩⟩
                                (full_offset + 1)))
                          else (pure false))
                      else (pure false))))))

/-- Whether a field is a byte string with its unique canonical RLP framing. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_bytes_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  if ((! f.is_list) : Bool)
  then
    (do
      (rlp_ref_framing_canonical f))
  else (pure false)

/-- Whether a field is the canonical RLP encoding of a non-negative
integer: minimal big-endian content with no leading zeros and the
exact matching evm_prefix. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_uint_canonical (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Bool := do
  if ((! (← (rlp_ref_bytes_canonical f))) : Bool)
  then (pure false)
  else
    if ((k_content_len == 0) : Bool)
    then (pure true)
    else
      (do
        (pure ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ (k_source_len - k_content_len))) != 0x00#8)))

/-- Decodes a string field of at most 32 bytes into a word. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_word (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let n := k_content_len
  if ((f.is_list || (RLP_WORD_LENGTH_LIMIT <b n)) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (slice_load_n ⟨_, ⟨_, f.source⟩⟩ (k_source_len - n) n)

/-- Decodes a canonical unsigned integer field into a word; throws
otherwise. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_uint_word (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  if ((← (rlp_ref_uint_canonical f)) : Bool)
  then (rlp_ref_word f)
  else sailThrow ((InvalidBlock RlpDecode))

/-- Decodes a canonical unsigned integer into the uint64 wire domain used by
EIP-2681 account nonces and EIP-4844 excess blob gas. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_uint64 (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  if (((! (← (rlp_ref_uint_canonical f))) || (RLP_UINT64_LENGTH_LIMIT <b k_content_len)) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else
    (do
      let content_length := k_content_len
      let ⟨_, ⟨_, content⟩⟩ := (rlp_ref_content f)
      let width : Nat := (Nat.mod content_length 9)
      (rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width))

/-- Decodes an arbitrary-width big-endian natural. -/
/- Type quantifiers: _reclimit : Nat, content_dependentWitness1 : Nat, content_dependentWitness0 :
  Nat, 0 ≤ content_dependentWitness0 ∧ 0 ≤ content_dependentWitness1, 0 ≤ _reclimit -/
def _rec_rlp_uint_content (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (_reclimit : Nat) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let content_length := content.len
      if ((content_length == 0) : Bool)
      then (pure 0)
      else
        (do
          let prefix_length := (content_length - 1)
          let evm_prefix' := (sub_slice content 0 prefix_length)
          (pure (((← (_rec_rlp_uint_content ⟨_, ⟨_, evm_prefix'⟩⟩ _reclimit_pred)) *i 256) + (BitVec.toNatInt
                (← (slice_byte ⟨_, ⟨_, content⟩⟩ prefix_length)))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Decodes an arbitrary-width big-endian natural. -/
/- Type quantifiers: content_dependentWitness1 : Nat, content_dependentWitness0 : Nat, 0 ≤
  content_dependentWitness0 ∧ 0 ≤ content_dependentWitness1 -/
def rlp_uint_content (content : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let content_dependentWitness0 := (content).1
  let content_dependentWitness1 := ((content).2).1
  let content := ((content).2).2
  let _measure := (content.len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_uint_content ⟨_, ⟨_, content⟩⟩ (_measure + 1))

/-- Decodes an arbitrary-width canonical RLP unsigned integer. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def rlp_ref_uint (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  if ((← (rlp_ref_uint_canonical f)) : Bool)
  then (rlp_uint_content (rlp_ref_content f))
  else sailThrow ((InvalidBlock RlpDecode))

