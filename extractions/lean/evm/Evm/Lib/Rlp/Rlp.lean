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
field reference retains its source, full span, and content span without
copying.

Malformed RLP is a normal invalid-block outcome: the decoders throw
`InvalidBlock(RlpDecode)` at the point of detection, caught once at the
guest top level. -/

def RLP_SHORT_LENGTH_LIMIT : Nat := 55

def RLP_WORD_LENGTH_LIMIT : Nat := WORD_BYTE_LENGTH

def RLP_UINT64_LENGTH_LIMIT : Nat := EIGHT_BYTE_LENGTH

def RLP_ENCODED_WORD_LENGTH : Nat := 33

def RLP_ENCODED_ADDRESS_LENGTH : Nat := 21

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 33 -/
def rlp_scratch_small_length (value : Nat) : rlp_scratch_length :=
  value

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧ 0 ≤ right -/
def rlp_scratch_length_add (left : Nat) (right : Nat) : rlp_scratch_length :=
  (left + right)

/-- Decrements a positive RLP length width. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 8 -/
def rlp_length_width_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/lib/rlp/rlp.sail:68.20-68.21"
  (pure (value - 1))

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 31 -/
def word_byte_count (index : Nat) : Nat :=
  (index + 1)

/-- Materializes a word's minimal big-endian byte sequence and its length. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def minimal_word_bytes (w : Nat) : ((List byte) × Nat) := Id.run do
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
            let remaining : Nat := ((word_shift_right ⟨remaining⟩ ⟨8⟩)).value
            let len : Nat := (word_byte_count byte_index)
            (len, out, remaining))
          else (len, out, remaining)
        (len, out, remaining)
    (pure loop_vars) ) : Id (Nat × (List (BitVec 8)) × Nat) )
  (pure (out, len))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 255 -/
def rlp_nat_length_byte (value : Nat) : byte :=
  (get_slice_int 8 value 0)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_byte_length_byte (value : Nat) : SailM byte := do
  assert (value ≤b 255) "sail/lib/rlp/rlp.sail:103.23-103.24"
  let length := value
  (pure (get_slice_int 8 length 0))

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_length_word (value : Nat) : SailM word := do
  let publicResult ← do
    (do
        let publicResult ← (word_of_nat_byte_count value)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_string_prefix (len : Nat) (first : byte) : SailM ((List byte) × Nat) := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure ([], 0))
  else
    (do
      if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
      then (pure ([(0x80#8 + (rlp_nat_length_byte len))], 1))
      else
        (do
          let (length_bytes, length_len) ← do
            (pure (minimal_word_bytes ((← (rlp_length_word len))).value))
          (pure (((0xB7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (1 + length_len)))))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_list_prefix (len : Nat) : SailM ((List byte) × Nat) := do
  if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure ([(0xC0#8 + (rlp_nat_length_byte len))], 1))
  else
    (do
      let (length_bytes, length_len) ← do
        (pure (minimal_word_bytes ((← (rlp_length_word len))).value))
      (pure (((0xF7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (1 + length_len))))

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def rlp_minimal_word_len (w : Nat) : Nat :=
  (word_byte_length w)

/- Type quantifiers: value : Nat, rlp_natural_increment_valid(value) -/
def rlp_natural_increment (value : Nat) : Nat :=
  (value + 1)

/-- Computes a natural's minimal big-endian byte length recursively. -/
/- Type quantifiers: _reclimit : Nat, n : Nat, rlp_natural_valid(n), 0 ≤ _reclimit -/
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
      else
        (pure (rlp_natural_increment
            (← (_rec_rlp_minimal_uint_len (Int.ediv n 256) _reclimit_pred)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Computes a natural's minimal big-endian byte length recursively. -/
/- Type quantifiers: n : Nat, rlp_natural_valid(n) -/
def rlp_minimal_uint_len (n : Nat) : SailM Nat := do
  let _measure := (n : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_minimal_uint_len n (_measure + 1))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_length_prefix_len (len : Nat) : SailM Nat := do
  if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure 1)
  else (pure (1 + (rlp_minimal_word_len ((← (rlp_length_word len))).value)))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_string_size (len : Nat) (first : byte) : SailM Nat := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure 1)
  else (pure ((← (rlp_length_prefix_len len)) + len))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_bytes_size (data : (List byte)) (len : Nat) : SailM Nat := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_string_size len first)

/- Type quantifiers: k_off : Nat, k_len : Nat, source_valid_range(k_off, k_len) -/
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

/- Type quantifiers: n : Nat, rlp_natural_valid(n) -/
def rlp_uint_nat_size (n : Nat) : SailM rlp_natural_size := do
  if ((n <b (2 ^i 256)) : Bool)
  then (pure (rlp_uint_word_size ((U256 n)).value))
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
def rlp_scratch_list_size (content_len : rlp_scratch_length) : SailM rlp_scratch_length := do
  (pure (rlp_scratch_length_add content_len
      (rlp_scratch_small_length (← (rlp_length_prefix_len content_len)))))

/-- Returns the materializable RLP width of a source-backed byte slice. -/
/- Type quantifiers: k_ex409620_ : Nat, k_ex409619_ : Nat, 0 ≤ k_ex409619_ ∧ 0 ≤ k_ex409620_ -/
def rlp_scratch_slice_size (data : EvmByteSlice) : SailM rlp_scratch_length := do
  let data := ((data).2).2
  (rlp_slice_size data)

/- Type quantifiers: len : Nat, source_valid_length(len) -/
def rlp_scratch_bytes_size (data : (List byte)) (len : Nat) : SailM rlp_scratch_length := do
  (rlp_bytes_size data len)

/- Type quantifiers: len : Nat, source_valid_length(len) -/
def rlp_write_raw_bytes (data : (List byte)) (len : Nat) : SailM Unit := do
  (scratch_push_bytes data len)

/- Type quantifiers: k_ex409634_ : Nat, k_ex409633_ : Nat, 0 ≤ k_ex409633_ ∧ 0 ≤ k_ex409634_ -/
def rlp_write_raw_slice (data : EvmByteSlice) : SailM Unit := do
  let data := ((data).2).2
  (scratch_push_slice ⟨_, ⟨_, data⟩⟩)

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_write_string_prefix (len : Nat) (first : byte) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_string_prefix len first)
  (scratch_push_bytes encoded_prefix prefix_len)

/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def rlp_write_list_prefix (content_len : Nat) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_list_prefix content_len)
  (scratch_push_bytes encoded_prefix prefix_len)

/- Type quantifiers: len : Nat, source_valid_length(len) -/
def rlp_write_bytes (data : (List byte)) (len : Nat) : SailM Unit := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_write_string_prefix len first)
  (scratch_push_bytes data len)

/-- Appends a source-backed byte slice as an RLP string. -/
/- Type quantifiers: k_ex409643_ : Nat, k_ex409642_ : Nat, 0 ≤ k_ex409642_ ∧ 0 ≤ k_ex409643_ -/
def rlp_write_slice (data : EvmByteSlice) : SailM Unit := do
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

/- Type quantifiers: n : Nat, rlp_natural_valid(n) -/
def rlp_write_uint_nat (n : Nat) : SailM Unit := do
  if ((n <b (2 ^i 256)) : Bool)
  then (rlp_write_uint_word ((U256 n)).value)
  else sailThrow ((InvalidBlock InvalidConfig))

/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_write_uint_u64 (n : Nat) : SailM Unit := do
  (rlp_write_uint_word ((U256 n)).value)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def rlp_write_word (w : word) : SailM Unit := do
  let w := (w).value
  (rlp_write_bytes (word_to_bytes32 ⟨w⟩) WORD_BYTE_LENGTH)

def rlp_write_addr (a : address) : SailM Unit := do
  (rlp_write_bytes (address_to_bytes a) ADDRESS_BYTE_LENGTH)

/-- Finishes a forward scratch encoding. -/
/- Type quantifiers: start : Nat, 0 ≤ start -/
def rlp_finish (start : source_pointer) : SailM EvmByteSlice := do
  (scratch_finish start)

/- Type quantifiers: _width : Nat, evm_prefix' : Nat, 1 ≤ _width ∧
  _width ≤ 8 ∧ 0 ≤ evm_prefix' ∧ evm_prefix' ≤ (2 ^ (8 * (_width - 1)) - 1) -/
def rlp_uint64_append (_width : Nat) (evm_prefix' : Nat) (next : byte) : Nat :=
  ((evm_prefix' *i 256) + (BitVec.toNatInt next))

/-- Decodes exactly `width` big-endian bytes into a bounded unsigned value. -/
/- Type quantifiers: _reclimit : Nat, k_ex409654_ : Nat, k_ex409653_ : Nat, width : Nat, 0 ≤ width
  ∧ width ≤ 8, 0 ≤ k_ex409653_ ∧ 0 ≤ k_ex409654_, 0 ≤ _reclimit -/
def _rec_rlp_uint64_width (content : EvmByteSlice) (width : Nat) (_reclimit : Nat) : SailM Nat := do
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
/- Type quantifiers: k_ex409659_ : Nat, k_ex409658_ : Nat, width : Nat, 0 ≤ width ∧ width ≤ 8, 0
  ≤ k_ex409658_ ∧ 0 ≤ k_ex409659_ -/
def rlp_uint64_width (content : EvmByteSlice) (width : Nat) : SailM Nat := do
  let content := ((content).2).2
  let _measure := (width : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width (_measure + 1))

/- Type quantifiers: k_ex409670_ : Nat, k_ex409669_ : Nat, start : Nat, stop : Nat, count : Nat, source_valid_length(start)
  ∧ source_valid_length(stop) ∧ 0 ≤ count ∧ count ≤ 8, 0 ≤ k_ex409669_ ∧
  0 ≤ k_ex409670_ -/
def rlp_ref_be_length (source : EvmByteSlice) (start : Nat) (stop : Nat) (count : Nat) : SailM Nat := do
  let source := ((source).2).2
  let source_length := source.len
  let start_value := start
  let stop_value := stop
  if ((stop_value <b start_value) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else
    (do
      if ((source_length <b stop_value) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else
        (do
          if (((stop_value - start_value) <b count) : Bool)
          then sailThrow ((InvalidBlock RlpDecode))
          else
            (do
              let content := (sub_slice source start_value count)
              (rlp_uint64_width ⟨_, ⟨_, content⟩⟩ count))))

/-- Tests a bounded expected byte sequence against a source position. -/
/- Type quantifiers: k_ex409679_ : Nat, k_ex409678_ : Nat, k_ex409677_ : Nat, 0 ≤ k_ex409677_ ∧
  0 ≤ k_ex409678_, 0 ≤ k_ex409679_ -/
def rlp_bytes_equal_at (expected : (List byte)) (source : EvmByteSlice) (start : source_pointer) : SailM Bool := do
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
                      (pure (equal && ((← (slice_byte ⟨_, ⟨_, source⟩⟩ current)) == b)))
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

/- Type quantifiers: k_ex409689_ : Nat, k_ex409688_ : Nat, pos : Nat, stop : Nat, source_valid_length(pos)
  ∧ source_valid_length(stop), 0 ≤ k_ex409688_ ∧ 0 ≤ k_ex409689_ -/
def rlp_ref_hdr (b : EvmByteSlice) (pos : Nat) (stop : Nat) : SailM (Bool × source_pointer × Nat) := do
  let b := ((b).2).2
  if ((! (pos <b stop)) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let h ← do (pure (BitVec.toNatInt (← (slice_byte ⟨_, ⟨_, b⟩⟩ pos))))
  if ((h <b 128) : Bool)
  then (pure (false, pos, 1))
  else
    (do
      let position := pos
      let stop_position := stop
      let length_start : Nat := (position + 1)
      if ((h <b 184) : Bool)
      then (pure (false, length_start, (h - 128)))
      else
        (do
          if ((h <b 192) : Bool)
          then
            (do
              let length_width : Nat := (h - 183)
              if (((stop_position - (position + 1)) <b length_width) : Bool)
              then sailThrow ((InvalidBlock RlpDecode))
              else (pure ())
              let content : Nat := ((position + 1) + length_width)
              (pure (false, content, (← (rlp_ref_be_length ⟨_, ⟨_, b⟩⟩ length_start
                    content length_width)))))
          else
            (do
              if ((h <b 248) : Bool)
              then (pure (true, length_start, (h - 192)))
              else
                (do
                  let length_width : Nat := (h - 247)
                  if (((stop_position - (position + 1)) <b length_width) : Bool)
                  then sailThrow ((InvalidBlock RlpDecode))
                  else (pure ())
                  let content : Nat := ((position + 1) + length_width)
                  (pure (true, content, (← (rlp_ref_be_length ⟨_, ⟨_, b⟩⟩ length_start
                        content length_width))))))))

/- Type quantifiers: k_ex409702_ : Bool, k_source_off : Nat, k_source_len : Nat, full_off : Nat, full_len
  : Nat, content_off : Nat, content_len : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ full_off ∧
  0 ≤ full_len ∧
  (full_off + full_len) ≤ k_source_len ∧
  0 ≤ content_off ∧ 0 ≤ content_len ∧ (content_off + content_len) ≤ k_source_len -/
def rlp_field_ref (source : (EvmByteSliceFields k_source_off k_source_len)) (is_list : Bool) (full_off : Nat) (full_len : Nat) (content_off : Nat) (content_len : Nat) : RlpFieldRef :=
  let fields : (RlpFieldRefFields k_source_off k_source_len full_off full_len content_off content_len) :=
    { source := source,
      is_list := is_list,
      full_off := full_off,
      full_len := full_len,
      content_off := content_off,
      content_len := content_len }
  ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩⟩⟩ : (Sigma fun
  (k_syn_source_off : Nat) =>
  (Sigma fun (k_syn_source_len : Nat) =>
  (Sigma fun (k_syn_full_off : Nat) =>
  (Sigma fun (k_syn_full_len : Nat) =>
  (Sigma fun (k_syn_content_off : Nat) =>
  (Sigma fun (k_syn_content_len : Nat) =>
  (RlpFieldRefFields k_syn_source_off k_syn_source_len k_syn_full_off k_syn_full_len k_syn_content_off k_syn_content_len)))))))) : (Sigma
  fun (k_syn_source_off : Nat) =>
  (Sigma fun (k_syn_source_len : Nat) =>
  (Sigma fun (k_syn_full_off : Nat) =>
  (Sigma fun (k_syn_full_len : Nat) =>
  (Sigma fun (k_syn_content_off : Nat) =>
  (Sigma fun (k_syn_content_len : Nat) =>
  (RlpFieldRefFields k_syn_source_off k_syn_source_len k_syn_full_off k_syn_full_len k_syn_content_off k_syn_content_len))))))))

/- Type quantifiers: k_ex409715_ : Bool, k_source_off : Nat, k_source_len : Nat, current : Nat, stop
  : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ current ∧ current ≤ stop ∧ stop ≤ k_source_len -/
def rlp_cursor (source : (EvmByteSliceFields k_source_off k_source_len)) (current : Nat) (stop : Nat) (valid : Bool) : RlpCursor :=
  let fields : (RlpCursorFields k_source_off k_source_len current stop) :=
    { source := source,
      current := current,
      stop := stop,
      valid := valid }
  ((⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩ : (Sigma fun (k_syn_source_off : Nat) =>
  (Sigma fun (k_syn_source_len : Nat) =>
  (Sigma fun (k_syn_current : Nat) =>
  (Sigma fun (k_syn_stop : Nat) =>
  (RlpCursorFields k_syn_source_off k_syn_source_len k_syn_current k_syn_stop)))))) : (Sigma fun
  (k_syn_source_off : Nat) =>
  (Sigma fun (k_syn_source_len : Nat) =>
  (Sigma fun (k_syn_current : Nat) =>
  (Sigma fun (k_syn_stop : Nat) =>
  (RlpCursorFields k_syn_source_off k_syn_source_len k_syn_current k_syn_stop))))))

/-- The invalid cursor; popping it throws `InvalidBlock(RlpDecode)`. -/
/- Type quantifiers: k_ex409723_ : Nat, k_ex409722_ : Nat, 0 ≤ k_ex409722_ ∧ 0 ≤ k_ex409723_ -/
def rlp_invalid_cursor (source : EvmByteSlice) : RlpCursor :=
  let source := ((source).2).2
  ((⟨_, ⟨_, ⟨_, ⟨_, (((((rlp_cursor source 0 0 false)).2).2).2).2⟩⟩⟩⟩ : (Sigma fun
  (k_source_off : Nat) =>
  (Sigma fun (k_source_len : Nat) =>
  (Sigma fun (k_current : Nat) =>
  (Sigma fun (k_stop : Nat) => (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) : (Sigma
  fun (k_source_off : Nat) =>
  (Sigma fun (k_source_len : Nat) =>
  (Sigma fun (k_current : Nat) =>
  (Sigma fun (k_stop : Nat) => (RlpCursorFields k_source_off k_source_len k_current k_stop))))))

/-- A cursor over the children of a byte sequence that must be exactly
one RLP list (e.g. a trie node). -/
/- Type quantifiers: k_ex409727_ : Nat, k_ex409726_ : Nat, 0 ≤ k_ex409726_ ∧ 0 ≤ k_ex409727_ -/
def rlp_node_cursor (node : EvmByteSlice) : SailM RlpCursor := do
  let node := ((node).2).2
  let node_length := node.len
  if ((node_length == 0) : Bool)
  then
    (pure ((rlp_invalid_cursor ⟨_, ⟨_, node⟩⟩) : (Sigma fun (k_source_off : Nat) =>
      (Sigma fun (k_source_len : Nat) =>
      (Sigma fun (k_current : Nat) =>
      (Sigma fun (k_stop : Nat) => (RlpCursorFields k_source_off k_source_len k_current k_stop)))))))
  else
    (do
      let node_stop := node_length
      let (is_list, content, content_len) ← do (rlp_ref_hdr ⟨_, ⟨_, node⟩⟩ 0 node_stop)
      let content_offset := content
      let payload_length := content_len
      if ((content_offset ≤b node_length) : Bool)
      then
        (if ((payload_length ≤b (node_length - content_offset)) : Bool)
        then
          (let content_stop := (content_offset + payload_length)
          if (is_list : Bool)
          then
            (if ((content_stop == node_stop) : Bool)
            then
              (pure ((⟨_, ⟨_, ⟨_, ⟨_, (((((rlp_cursor node content content_stop true)).2).2).2).2⟩⟩⟩⟩ : (Sigma
                fun (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop)))))) : (Sigma fun
                (k_source_off : Nat) =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop)))))))
            else
              (pure ((rlp_invalid_cursor ⟨_, ⟨_, node⟩⟩) : (Sigma fun (k_source_off : Nat)
                =>
                (Sigma fun (k_source_len : Nat) =>
                (Sigma fun (k_current : Nat) =>
                (Sigma fun (k_stop : Nat) =>
                (RlpCursorFields k_source_off k_source_len k_current k_stop))))))))
          else
            (pure ((rlp_invalid_cursor ⟨_, ⟨_, node⟩⟩) : (Sigma fun (k_source_off : Nat) =>
              (Sigma fun (k_source_len : Nat) =>
              (Sigma fun (k_current : Nat) =>
              (Sigma fun (k_stop : Nat) =>
              (RlpCursorFields k_source_off k_source_len k_current k_stop))))))))
        else
          (pure ((rlp_invalid_cursor ⟨_, ⟨_, node⟩⟩) : (Sigma fun (k_source_off : Nat) =>
            (Sigma fun (k_source_len : Nat) =>
            (Sigma fun (k_current : Nat) =>
            (Sigma fun (k_stop : Nat) =>
            (RlpCursorFields k_source_off k_source_len k_current k_stop))))))))
      else
        (pure ((rlp_invalid_cursor ⟨_, ⟨_, node⟩⟩) : (Sigma fun (k_source_off : Nat) =>
          (Sigma fun (k_source_len : Nat) =>
          (Sigma fun (k_current : Nat) =>
          (Sigma fun (k_stop : Nat) => (RlpCursorFields k_source_off k_source_len k_current k_stop))))))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_full_off : Nat, k_full_len : Nat, k_content_off
  : Nat, k_content_len : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ k_full_off ∧
  0 ≤ k_full_len ∧
  (k_full_off + k_full_len) ≤ k_source_len ∧
  0 ≤ k_content_off ∧ 0 ≤ k_content_len ∧ (k_content_off + k_content_len) ≤ k_source_len -/
def rlp_ref_cursor (f : (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len)) : SailM (Sigma
  fun (k_cursor_source_off : Nat) =>
  (Sigma fun (k_cursor_source_len : Nat) =>
  (Sigma fun (k_cursor_current : Nat) =>
  (Sigma fun (k_cursor_stop : Nat) =>
  (RlpCursorFields k_cursor_source_off k_cursor_source_len k_cursor_current k_cursor_stop))))) := do
  if (f.is_list : Bool)
  then
    (do
      let content_offset := k_content_off
      let content_length := k_content_len
      let source_length := k_source_len
      let stop := (content_offset + content_length)
      let ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩ :=
        (rlp_cursor f.source content_offset stop true)
      let cursor_span := (cursor.stop - cursor.current)
      let full_length := k_full_len
      if ((cursor_span ≤b full_length) : Bool)
      then
        (pure ((⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩ : (Sigma fun (k_cursor_source_off : Nat)
          =>
          (Sigma fun (k_cursor_source_len : Nat) =>
          (Sigma fun (k_cursor_current : Nat) =>
          (Sigma fun (k_cursor_stop : Nat) =>
          (RlpCursorFields k_cursor_source_off k_cursor_source_len k_cursor_current k_cursor_stop)))))) : (Sigma
          fun (k_cursor_source_off : Nat) =>
          (Sigma fun (k_cursor_source_len : Nat) =>
          (Sigma fun (k_cursor_current : Nat) =>
          (Sigma fun (k_cursor_stop : Nat) =>
          (RlpCursorFields k_cursor_source_off k_cursor_source_len k_cursor_current k_cursor_stop)))))))
      else
        (do
          sailThrow ((InvalidBlock RlpDecode))))
  else
    (do
      sailThrow ((InvalidBlock RlpDecode)))

/-- Whether the cursor has consumed all children. -/
/- Type quantifiers: k_ex409753_ : Nat, k_ex409752_ : Nat, k_ex409751_ : Nat, k_ex409750_ : Nat, 0
  ≤ k_ex409750_ ∧ 0 ≤ k_ex409751_ ∧
  0 ≤ k_ex409752_ ∧ k_ex409752_ ≤ k_ex409753_ ∧ k_ex409753_ ≤ k_ex409751_ -/
def rlp_cursor_empty (cursor : RlpCursor) : Bool :=
  let cursor := ((((cursor).2).2).2).2
  (cursor.valid && (cursor.current == cursor.stop))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_current : Nat, k_stop : Nat, source_valid_range(k_source_off, k_source_len)
  ∧ 0 ≤ k_current ∧ k_current ≤ k_stop ∧ k_stop ≤ k_source_len -/
def rlp_cursor_pop (cursor : (RlpCursorFields k_source_off k_source_len k_current k_stop)) : SailM (Sigma
  fun (k_content : Nat) =>
  (Sigma fun (k_content_len : Nat) =>
  (Sigma fun (k_next : Nat) =>
  (Sigma fun (k_full_len : Nat) =>
  ((RlpFieldRefFields k_source_off k_source_len k_current k_full_len k_content k_content_len) × (RlpCursorFields k_source_off k_source_len k_next k_stop)))))) := do
  let current := k_current
  let stop := k_stop
  if (((! cursor.valid) || (! (current <b stop))) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let (is_list, content_value, content_len_value) ← do
    (rlp_ref_hdr ⟨_, ⟨_, cursor.source⟩⟩ current stop)
  let content := content_value
  let content_len := content_len_value
  let content_offset := content
  let payload_length := content_len
  let stop_offset := stop
  if ((stop_offset <b content_offset) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  if (((stop_offset - content_offset) <b payload_length) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let next := (content_offset + payload_length)
  let current_offset := current
  let next_offset := next
  if ((current_offset <b next_offset) : Bool)
  then
    (let full_len := (next_offset - current_offset)
    let field : (RlpFieldRefFields k_source_off k_source_len k_current full_len content content_len) :=
      { source := cursor.source,
        is_list := is_list,
        full_off := current,
        full_len := full_len,
        content_off := content,
        content_len := content_len }
    let advanced : (RlpCursorFields k_source_off k_source_len next k_stop) :=
      { source := cursor.source,
        current := next,
        stop := stop,
        valid := true }
    (pure ((⟨_, ⟨_, ⟨_, ⟨_, (field, advanced)⟩⟩⟩⟩ : (Sigma fun (k_syn_content : Nat)
      =>
      (Sigma fun (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_next : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      ((RlpFieldRefFields k_source_off k_source_len k_current k_syn_full_len k_syn_content k_syn_content_len) × (RlpCursorFields k_source_off k_source_len k_syn_next k_stop))))))) : (Sigma
      fun (k_syn_content : Nat) =>
      (Sigma fun (k_syn_content_len : Nat) =>
      (Sigma fun (k_syn_next : Nat) =>
      (Sigma fun (k_syn_full_len : Nat) =>
      ((RlpFieldRefFields k_source_off k_source_len k_current k_syn_full_len k_syn_content k_syn_content_len) × (RlpCursorFields k_source_off k_source_len k_syn_next k_stop)))))))))
  else
    (do
      sailThrow ((InvalidBlock RlpDecode)))

/-- Asserts the cursor is exhausted — trailing bytes are a decode
error. -/
/- Type quantifiers: k_ex409777_ : Nat, k_ex409776_ : Nat, k_ex409775_ : Nat, k_ex409774_ : Nat, 0
  ≤ k_ex409774_ ∧ 0 ≤ k_ex409775_ ∧
  0 ≤ k_ex409776_ ∧ k_ex409776_ ≤ k_ex409777_ ∧ k_ex409777_ ≤ k_ex409775_ -/
def rlp_cursor_expect_end (cursor : RlpCursor) : SailM Unit := do
  let cursor := ((((cursor).2).2).2).2
  if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
  then (pure ())
  else sailThrow ((InvalidBlock RlpDecode))

/-- Decodes a byte sequence that must be exactly one RLP item. -/
/- Type quantifiers: k_ex409781_ : Nat, k_ex409780_ : Nat, 0 ≤ k_ex409780_ ∧ 0 ≤ k_ex409781_ -/
def rlp_single_ref (item : EvmByteSlice) : SailM RlpFieldRef := do
  let item := ((item).2).2
  let item_length := item.len
  if ((item_length == 0) : Bool)
  then
    (do
      sailThrow ((InvalidBlock RlpDecode)))
  else
    (do
      let item_stop := item_length
      let (isl, cs, cl) ← do (rlp_ref_hdr ⟨_, ⟨_, item⟩⟩ 0 item_stop)
      let content_offset := cs
      let content_length := cl
      if ((content_offset >b item_length) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      if ((content_length != (item_length - content_offset)) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      (pure ((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (((((((rlp_field_ref item isl 0 item.len cs cl)).2).2).2).2).2).2⟩⟩⟩⟩⟩⟩ : (Sigma
        fun (k_source_off : Nat) =>
        (Sigma fun (k_source_len : Nat) =>
        (Sigma fun (k_full_off : Nat) =>
        (Sigma fun (k_full_len : Nat) =>
        (Sigma fun (k_content_off : Nat) =>
        (Sigma fun (k_content_len : Nat) =>
        (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len)))))))) : (Sigma
        fun (k_source_off : Nat) =>
        (Sigma fun (k_source_len : Nat) =>
        (Sigma fun (k_full_off : Nat) =>
        (Sigma fun (k_full_len : Nat) =>
        (Sigma fun (k_content_off : Nat) =>
        (Sigma fun (k_content_len : Nat) =>
        (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len))))))))))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_full_off : Nat, k_full_len : Nat, k_content_off
  : Nat, k_content_len : Nat, source_valid_range(k_source_off, k_source_len) ∧
  0 ≤ k_full_off ∧
  0 ≤ k_full_len ∧
  (k_full_off + k_full_len) ≤ k_source_len ∧
  0 ≤ k_content_off ∧ 0 ≤ k_content_len ∧ (k_content_off + k_content_len) ≤ k_source_len -/
def rlp_ref_content (f : (RlpFieldRefFields k_source_off k_source_len k_full_off k_full_len k_content_off k_content_len)) : (EvmByteSliceFields (k_source_off + k_content_off) k_content_len) :=
  (sub_slice f.source k_content_off k_content_len)

/-- The full encoded span of a field. -/
/- Type quantifiers: k_ex409807_ : Nat, k_ex409806_ : Nat, k_ex409805_ : Nat, k_ex409804_ : Nat, k_ex409803_
  : Nat, k_ex409802_ : Nat, 0 ≤ k_ex409802_ ∧ 0 ≤ k_ex409803_ ∧
  0 ≤ k_ex409804_ ∧
  0 ≤ k_ex409805_ ∧
  (k_ex409804_ + k_ex409805_) ≤ k_ex409803_ ∧
  0 ≤ k_ex409806_ ∧ 0 ≤ k_ex409807_ ∧ (k_ex409806_ + k_ex409807_) ≤ k_ex409803_ -/
def rlp_ref_full (f : RlpFieldRef) : EvmByteSlice :=
  let f := ((((((f).2).2).2).2).2).2
  ((⟨_, ⟨_, (sub_slice f.source f.full_off f.full_len)⟩⟩ : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))

/-- Whether a field uses the unique canonical RLP framing for its payload. -/
/- Type quantifiers: k_ex409819_ : Nat, k_ex409818_ : Nat, k_ex409817_ : Nat, k_ex409816_ : Nat, k_ex409815_
  : Nat, k_ex409814_ : Nat, 0 ≤ k_ex409814_ ∧ 0 ≤ k_ex409815_ ∧
  0 ≤ k_ex409816_ ∧
  0 ≤ k_ex409817_ ∧
  (k_ex409816_ + k_ex409817_) ≤ k_ex409815_ ∧
  0 ≤ k_ex409818_ ∧ 0 ≤ k_ex409819_ ∧ (k_ex409818_ + k_ex409819_) ≤ k_ex409815_ -/
def rlp_ref_framing_canonical (f : RlpFieldRef) : SailM Bool := do
  let f := ((((((f).2).2).2).2).2).2
  let n := f.content_len
  let payload_length := n
  let full_length := f.full_len
  let fo := f.full_off
  let full_offset := fo
  let co := f.content_off
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
            (pure ((full_length == (payload_length + 1)) && ((← (slice_byte
                      ⟨_, ⟨_, f.source⟩⟩ fo)) == (0xC0#8 + (rlp_nat_length_byte n)))))
          else
            (do
              let (len_bytes, length_width) ← do
                (pure (minimal_word_bytes ((← (rlp_length_word n))).value))
              (pure ((full_length == ((1 + length_width) + payload_length)) && (((← (slice_byte
                          ⟨_, ⟨_, f.source⟩⟩ fo)) == (0xF7#8 + (rlp_nat_length_byte
                          length_width))) && (← (rlp_bytes_equal_at len_bytes
                        ⟨_, ⟨_, f.source⟩⟩ (full_offset + 1))))))))
      else
        (do
          if ((payload_length == 0) : Bool)
          then
            (pure ((full_length == 1) && ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ fo)) == 0x80#8)))
          else
            (do
              let first ← do (slice_byte ⟨_, ⟨_, f.source⟩⟩ co)
              if (((payload_length == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
              then
                (pure ((full_length == 1) && ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ fo)) == first)))
              else
                (do
                  if ((n ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
                  then
                    (pure ((full_length == (payload_length + 1)) && ((← (slice_byte
                              ⟨_, ⟨_, f.source⟩⟩ fo)) == (0x80#8 + (rlp_nat_length_byte n)))))
                  else
                    (do
                      let (len_bytes, length_width) ← do
                        (pure (minimal_word_bytes ((← (rlp_length_word n))).value))
                      (pure ((full_length == ((1 + length_width) + payload_length)) && (((← (slice_byte
                                  ⟨_, ⟨_, f.source⟩⟩ fo)) == (0xB7#8 + (rlp_nat_length_byte
                                  length_width))) && (← (rlp_bytes_equal_at len_bytes
                                ⟨_, ⟨_, f.source⟩⟩ (full_offset + 1)))))))))))

/- Type quantifiers: k_ex409831_ : Nat, k_ex409830_ : Nat, k_ex409829_ : Nat, k_ex409828_ : Nat, k_ex409827_
  : Nat, k_ex409826_ : Nat, 0 ≤ k_ex409826_ ∧ 0 ≤ k_ex409827_ ∧
  0 ≤ k_ex409828_ ∧
  0 ≤ k_ex409829_ ∧
  (k_ex409828_ + k_ex409829_) ≤ k_ex409827_ ∧
  0 ≤ k_ex409830_ ∧ 0 ≤ k_ex409831_ ∧ (k_ex409830_ + k_ex409831_) ≤ k_ex409827_ -/
def rlp_ref_bytes_canonical (f : RlpFieldRef) : SailM Bool := do
  let f := ((((((f).2).2).2).2).2).2
  (pure ((! f.is_list) && (← (rlp_ref_framing_canonical
          ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩))))

/-- Whether a field is the canonical RLP encoding of a non-negative
integer: minimal big-endian content with no leading zeros and the
exact matching evm_prefix. -/
/- Type quantifiers: k_ex409843_ : Nat, k_ex409842_ : Nat, k_ex409841_ : Nat, k_ex409840_ : Nat, k_ex409839_
  : Nat, k_ex409838_ : Nat, 0 ≤ k_ex409838_ ∧ 0 ≤ k_ex409839_ ∧
  0 ≤ k_ex409840_ ∧
  0 ≤ k_ex409841_ ∧
  (k_ex409840_ + k_ex409841_) ≤ k_ex409839_ ∧
  0 ≤ k_ex409842_ ∧ 0 ≤ k_ex409843_ ∧ (k_ex409842_ + k_ex409843_) ≤ k_ex409839_ -/
def rlp_ref_uint_canonical (f : RlpFieldRef) : SailM Bool := do
  let f := ((((((f).2).2).2).2).2).2
  if ((! (← (rlp_ref_bytes_canonical ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩))) : Bool)
  then (pure false)
  else
    (pure ((f.content_len == 0) || ((← (slice_byte ⟨_, ⟨_, f.source⟩⟩ f.content_off)) != 0x00#8)))

/-- Returns a canonical byte-string field of exactly `len` bytes. -/
/- Type quantifiers: k_ex409856_ : Nat, k_ex409855_ : Nat, k_ex409854_ : Nat, k_ex409853_ : Nat, k_ex409852_
  : Nat, k_ex409851_ : Nat, k_ex409850_ : Nat, 0 ≤ k_ex409850_ ∧ 0 ≤ k_ex409851_ ∧
  0 ≤ k_ex409852_ ∧
  0 ≤ k_ex409853_ ∧
  (k_ex409852_ + k_ex409853_) ≤ k_ex409851_ ∧
  0 ≤ k_ex409854_ ∧ 0 ≤ k_ex409855_ ∧ (k_ex409854_ + k_ex409855_) ≤ k_ex409851_, 0 ≤
  k_ex409856_ -/
def rlp_ref_fixed_bytes (f : RlpFieldRef) (len : Nat) : SailM EvmByteSlice := do
  let f := ((((((f).2).2).2).2).2).2
  if (((← (rlp_ref_bytes_canonical ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)) && (f.content_len == len)) : Bool)
  then
    (pure ((⟨_, ⟨_, (rlp_ref_content f)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      sailThrow ((InvalidBlock RlpDecode)))

/-- Decodes a string field of at most 32 bytes into a word. -/
/- Type quantifiers: k_ex409868_ : Nat, k_ex409867_ : Nat, k_ex409866_ : Nat, k_ex409865_ : Nat, k_ex409864_
  : Nat, k_ex409863_ : Nat, 0 ≤ k_ex409863_ ∧ 0 ≤ k_ex409864_ ∧
  0 ≤ k_ex409865_ ∧
  0 ≤ k_ex409866_ ∧
  (k_ex409865_ + k_ex409866_) ≤ k_ex409864_ ∧
  0 ≤ k_ex409867_ ∧ 0 ≤ k_ex409868_ ∧ (k_ex409867_ + k_ex409868_) ≤ k_ex409864_ -/
def rlp_ref_word (f : RlpFieldRef) : SailM word := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    let n := f.content_len
    if ((f.is_list || (RLP_WORD_LENGTH_LIMIT <b n)) : Bool)
    then sailThrow ((InvalidBlock RlpDecode))
    else
      (do
          let publicResult ← (slice_load_n ⟨_, ⟨_, f.source⟩⟩ f.content_off n)
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Decodes a canonical unsigned integer field into a word; throws
otherwise. -/
/- Type quantifiers: k_ex409880_ : Nat, k_ex409879_ : Nat, k_ex409878_ : Nat, k_ex409877_ : Nat, k_ex409876_
  : Nat, k_ex409875_ : Nat, 0 ≤ k_ex409875_ ∧ 0 ≤ k_ex409876_ ∧
  0 ≤ k_ex409877_ ∧
  0 ≤ k_ex409878_ ∧
  (k_ex409877_ + k_ex409878_) ≤ k_ex409876_ ∧
  0 ≤ k_ex409879_ ∧ 0 ≤ k_ex409880_ ∧ (k_ex409879_ + k_ex409880_) ≤ k_ex409876_ -/
def rlp_ref_uint_word (f : RlpFieldRef) : SailM word := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    if ((← (rlp_ref_uint_canonical ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)) : Bool)
    then
      (do
          let publicResult ← (rlp_ref_word ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)
          pure ((publicResult).value))
    else sailThrow ((InvalidBlock RlpDecode))
  pure (⟨publicResult⟩)

/-- Decodes a canonical unsigned integer into the uint64 wire domain used by
EIP-2681 account nonces and EIP-4844 excess blob gas. -/
/- Type quantifiers: k_ex409892_ : Nat, k_ex409891_ : Nat, k_ex409890_ : Nat, k_ex409889_ : Nat, k_ex409888_
  : Nat, k_ex409887_ : Nat, 0 ≤ k_ex409887_ ∧ 0 ≤ k_ex409888_ ∧
  0 ≤ k_ex409889_ ∧
  0 ≤ k_ex409890_ ∧
  (k_ex409889_ + k_ex409890_) ≤ k_ex409888_ ∧
  0 ≤ k_ex409891_ ∧ 0 ≤ k_ex409892_ ∧ (k_ex409891_ + k_ex409892_) ≤ k_ex409888_ -/
def rlp_ref_uint64 (f : RlpFieldRef) : SailM ssz_uint := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    if (((! (← (rlp_ref_uint_canonical ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩))) || (RLP_UINT64_LENGTH_LIMIT <b f.content_len)) : Bool)
    then sailThrow ((InvalidBlock RlpDecode))
    else (pure ())
    let content_length := f.content_len
    let content := (rlp_ref_content f)
    let width : Nat := (Nat.mod content_length 9)
    (rlp_uint64_width ⟨_, ⟨_, content⟩⟩ width)
  pure (⟨publicResult⟩)

/-- Decodes an arbitrary-width big-endian natural. -/
/- Type quantifiers: _reclimit : Nat, k_ex409896_ : Nat, k_ex409895_ : Nat, 0 ≤ k_ex409895_ ∧
  0 ≤ k_ex409896_, 0 ≤ _reclimit -/
def _rec_rlp_uint_content (content : EvmByteSlice) (_reclimit : Nat) : SailM Nat := do
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
/- Type quantifiers: k_ex409901_ : Nat, k_ex409900_ : Nat, 0 ≤ k_ex409900_ ∧ 0 ≤ k_ex409901_ -/
def rlp_uint_content (content : EvmByteSlice) : SailM Nat := do
  let content := ((content).2).2
  let _measure := (content.len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_uint_content ⟨_, ⟨_, content⟩⟩ (_measure + 1))

/-- Decodes an arbitrary-width canonical RLP unsigned integer. -/
/- Type quantifiers: k_ex409914_ : Nat, k_ex409913_ : Nat, k_ex409912_ : Nat, k_ex409911_ : Nat, k_ex409910_
  : Nat, k_ex409909_ : Nat, 0 ≤ k_ex409909_ ∧ 0 ≤ k_ex409910_ ∧
  0 ≤ k_ex409911_ ∧
  0 ≤ k_ex409912_ ∧
  (k_ex409911_ + k_ex409912_) ≤ k_ex409910_ ∧
  0 ≤ k_ex409913_ ∧ 0 ≤ k_ex409914_ ∧ (k_ex409913_ + k_ex409914_) ≤ k_ex409910_ -/
def rlp_ref_uint (f : RlpFieldRef) : SailM Nat := do
  let f := ((((((f).2).2).2).2).2).2
  if ((← (rlp_ref_uint_canonical ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)) : Bool)
  then (rlp_uint_content ⟨_, ⟨_, (rlp_ref_content f)⟩⟩)
  else sailThrow ((InvalidBlock RlpDecode))

/-- Typed RLP boundaries document and enforce the field's protocol domain. -/
/- Type quantifiers: k_ex409926_ : Nat, k_ex409925_ : Nat, k_ex409924_ : Nat, k_ex409923_ : Nat, k_ex409922_
  : Nat, k_ex409921_ : Nat, 0 ≤ k_ex409921_ ∧ 0 ≤ k_ex409922_ ∧
  0 ≤ k_ex409923_ ∧
  0 ≤ k_ex409924_ ∧
  (k_ex409923_ + k_ex409924_) ≤ k_ex409922_ ∧
  0 ≤ k_ex409925_ ∧ 0 ≤ k_ex409926_ ∧ (k_ex409925_ + k_ex409926_) ≤ k_ex409922_ -/
def rlp_ref_account_nonce (f : RlpFieldRef) : SailM account_nonce := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    (do
        let publicResult ← (rlp_ref_uint64 ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409938_ : Nat, k_ex409937_ : Nat, k_ex409936_ : Nat, k_ex409935_ : Nat, k_ex409934_
  : Nat, k_ex409933_ : Nat, 0 ≤ k_ex409933_ ∧ 0 ≤ k_ex409934_ ∧
  0 ≤ k_ex409935_ ∧
  0 ≤ k_ex409936_ ∧
  (k_ex409935_ + k_ex409936_) ≤ k_ex409934_ ∧
  0 ≤ k_ex409937_ ∧ 0 ≤ k_ex409938_ ∧ (k_ex409937_ + k_ex409938_) ≤ k_ex409934_ -/
def rlp_ref_block_number (f : RlpFieldRef) : SailM block_number := do
  let f := ((((((f).2).2).2).2).2).2
  (rlp_ref_uint ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)

/-- Decodes block blob gas after enforcing the supported schedule bound. -/
/- Type quantifiers: k_ex409950_ : Nat, k_ex409949_ : Nat, k_ex409948_ : Nat, k_ex409947_ : Nat, k_ex409946_
  : Nat, k_ex409945_ : Nat, 0 ≤ k_ex409945_ ∧ 0 ≤ k_ex409946_ ∧
  0 ≤ k_ex409947_ ∧
  0 ≤ k_ex409948_ ∧
  (k_ex409947_ + k_ex409948_) ≤ k_ex409946_ ∧
  0 ≤ k_ex409949_ ∧ 0 ≤ k_ex409950_ ∧ (k_ex409949_ + k_ex409950_) ≤ k_ex409946_ -/
def rlp_ref_blob_gas_used (f : RlpFieldRef) : SailM blob_gas_used := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    let value ← do
      (do
          let publicResult ← (rlp_ref_uint64
          ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)
          pure ((publicResult).value))
    if ((value ≤b (21 *i (2 ^i 17))) : Bool)
    then (pure value)
    else sailThrow ((InvalidBlock RlpDecode))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409962_ : Nat, k_ex409961_ : Nat, k_ex409960_ : Nat, k_ex409959_ : Nat, k_ex409958_
  : Nat, k_ex409957_ : Nat, 0 ≤ k_ex409957_ ∧ 0 ≤ k_ex409958_ ∧
  0 ≤ k_ex409959_ ∧
  0 ≤ k_ex409960_ ∧
  (k_ex409959_ + k_ex409960_) ≤ k_ex409958_ ∧
  0 ≤ k_ex409961_ ∧ 0 ≤ k_ex409962_ ∧ (k_ex409961_ + k_ex409962_) ≤ k_ex409958_ -/
def rlp_ref_excess_blob_gas (f : RlpFieldRef) : SailM excess_blob_gas := do
  let f := ((((((f).2).2).2).2).2).2
  let publicResult ← do
    (do
        let publicResult ← (rlp_ref_uint64 ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409974_ : Nat, k_ex409973_ : Nat, k_ex409972_ : Nat, k_ex409971_ : Nat, k_ex409970_
  : Nat, k_ex409969_ : Nat, 0 ≤ k_ex409969_ ∧ 0 ≤ k_ex409970_ ∧
  0 ≤ k_ex409971_ ∧
  0 ≤ k_ex409972_ ∧
  (k_ex409971_ + k_ex409972_) ≤ k_ex409970_ ∧
  0 ≤ k_ex409973_ ∧ 0 ≤ k_ex409974_ ∧ (k_ex409973_ + k_ex409974_) ≤ k_ex409970_ -/
def rlp_ref_chain_identifier (f : RlpFieldRef) : SailM chain_identifier := do
  let f := ((((((f).2).2).2).2).2).2
  (rlp_ref_uint ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, f⟩⟩⟩⟩⟩⟩)

