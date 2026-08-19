import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Kernel.Scratch

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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # RLP encoding

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

Malformed RLP terminates validation with `fatal_error(RlpDecode)` at the
point where the invalid encoding is detected. -/

abbrev RLP_SHORT_LENGTH_LIMIT : Nat := 55

abbrev RLP_WORD_LENGTH_LIMIT : Nat := 32

abbrev RLP_UINT64_LENGTH_LIMIT : Nat := 8

abbrev RLP_ENCODED_WORD_LENGTH : Nat := 33

abbrev RLP_ENCODED_ADDRESS_LENGTH : Nat := 21

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 33 -/
def rlp_scratch_small_length (value : Nat) : Nat :=
  value

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧ 0 ≤ right -/
def rlp_scratch_length_add (left : Nat) (right : Nat) : SailM Nat := do
  if ((right ≤b (((2 ^i 32) - 1) -i left)) : Bool)
  then (pure (left + right))
  else (fatal_error RlpDecode)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 255 -/
def rlp_nat_length_byte (value : Nat) : (BitVec 8) :=
  (get_slice_int 8 value 0)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_byte_length_byte (value : Nat) : SailM (BitVec 8) := do
  assert (value ≤b 255) "sail/lib/rlp/encoding.sail:75.23-75.24"
  let length := value
  (pure (get_slice_int 8 length 0))

/- Type quantifiers: value : Nat, 0 ≤ value -/
def rlp_length_word (value : Nat) : SailM Nat := do
  (word_of_nat_byte_count value)

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
      else
        (do
          let remaining_length ← do (_rec_rlp_minimal_uint_len (n / 256) _reclimit_pred)
          (pure (rlp_natural_increment remaining_length))))
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
  else
    (do
      let length_word ← do (rlp_length_word len)
      (pure (1 + (rlp_minimal_word_len length_word))))

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_string_size (len : Nat) (first : (BitVec 8)) : SailM Nat := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure 1)
  else (pure ((← (rlp_length_prefix_len len)) + len))

/- Type quantifiers: k_off : Nat, k_len : Nat, (source_valid_range k_off k_len) -/
def rlp_input_slice_size (data : (StatelessInputSliceFields k_off k_len)) : SailM Nat := do
  let len := k_len
  let first ← do
    if ((len == 0) : Bool)
    then (pure 0x00#8)
    else (stateless_input_slice_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_string_size len first)

/-- Returns the materializable RLP width of a byte slice after its caller has
selected the backing region and loaded the first byte. -/
/- Type quantifiers: k_ex607727_ : Nat, 0 ≤ k_ex607727_ ∧ k_ex607727_ ≤ (2 ^ 32 - 1) -/
def rlp_materialized_slice_size (length : Nat) (first : (BitVec 8)) : SailM Nat := do
  if (((length == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure 1)
  else
    (do
      let prefix_size ← do (rlp_length_prefix_len length)
      let prefix_length := (rlp_scratch_small_length prefix_size)
      (rlp_scratch_length_add length prefix_length))

/-- Returns the materializable RLP width of a scratch-backed byte slice. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_scratch_region_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (scratch_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_materialized_slice_size data.len first)

/-- Returns the materializable RLP width of a retained log-data slice. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_log_data_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (log_data_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_materialized_slice_size data.len first)

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
  then
    (let word := (u256 n)
    (pure (rlp_uint_word_size word)))
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
/- Type quantifiers: content_len : Nat, 0 ≤ content_len ∧ content_len ≤ (2 ^ 32 - 1) -/
def rlp_scratch_list_size (content_len : Nat) : SailM Nat := do
  let prefix_size ← do (rlp_length_prefix_len content_len)
  let prefix_length := (rlp_scratch_small_length prefix_size)
  (rlp_scratch_length_add content_len prefix_length)

/-- Returns the materializable RLP width of a source-backed byte slice. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_input_scratch_slice_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (stateless_input_slice_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_materialized_slice_size data.len first)

/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_scratch_scratch_slice_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  (rlp_scratch_region_size ⟨_, ⟨_, data⟩⟩)

/-- Sizes a retained log-data slice for the materializable overload. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_log_scratch_slice_size (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) : SailM Nat := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  (rlp_log_data_size ⟨_, ⟨_, data⟩⟩)

/- Type quantifiers: len : Nat, 0 ≤ len -/
def rlp_write_string_prefix (len : Nat) (first : (BitVec 8)) : SailM Unit := do
  if (((len == 1) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure ())
  else
    (do
      if ((len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
      then
        (do
          let length_byte := (rlp_nat_length_byte len)
          let evm_prefix' := (0x80#8 + length_byte)
          (scratch_push_byte evm_prefix'))
      else
        (do
          let length_word ← do (rlp_length_word len)
          let length_len := (rlp_minimal_word_len length_word)
          let length_byte := (rlp_nat_length_byte length_len)
          let evm_prefix' := (0xB7#8 + length_byte)
          (scratch_push_byte evm_prefix')
          (scratch_push_word_be length_word length_len)))

/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def rlp_write_list_prefix (content_len : Nat) : SailM Unit := do
  if ((content_len ≤b RLP_SHORT_LENGTH_LIMIT) : Bool)
  then
    (do
      let length_byte := (rlp_nat_length_byte content_len)
      let evm_prefix' := (0xC0#8 + length_byte)
      (scratch_push_byte evm_prefix'))
  else
    (do
      let length_word ← do (rlp_length_word content_len)
      let length_len := (rlp_minimal_word_len length_word)
      let length_byte := (rlp_nat_length_byte length_len)
      let evm_prefix' := (0xF7#8 + length_byte)
      (scratch_push_byte evm_prefix')
      (scratch_push_word_be length_word length_len))

/-- Appends a source-backed byte slice as an RLP string. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_write_input_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (stateless_input_slice_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_write_string_prefix data.len first)
  (stateless_input_scratch_push_slice ⟨_, ⟨_, data⟩⟩)

/-- Appends a scratch-backed byte slice as an RLP string. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_write_scratch_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (scratch_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_write_string_prefix data.len first)
  (scratch_scratch_push_slice ⟨_, ⟨_, data⟩⟩)

/-- Appends a retained log-data slice as an RLP string. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def rlp_write_log_data_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let first ← do
    if ((data.len == 0) : Bool)
    then (pure 0x00#8)
    else (log_data_byte ⟨_, ⟨_, data⟩⟩ 0)
  (rlp_write_string_prefix data.len first)
  (log_data_scratch_push_slice ⟨_, ⟨_, data⟩⟩)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def rlp_write_uint_word (w : Nat) : SailM Unit := do
  let len := (rlp_minimal_word_len w)
  let first :=
    if ((len == 1) : Bool)
    then (word_low_byte w)
    else 0x00#8
  (rlp_write_string_prefix len first)
  let word := (u256 w)
  (scratch_push_word_be word len)

/- Type quantifiers: n : Nat, (rlp_natural_valid n) -/
def rlp_write_uint_nat (n : Nat) : SailM Unit := do
  if ((n <b (2 ^i 256)) : Bool)
  then
    (do
      let word := (u256 n)
      (rlp_write_uint_word word))
  else (fatal_error InvalidConfig)

/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_write_uint_u64 (n : Nat) : SailM Unit := do
  let word := (u256 n)
  (rlp_write_uint_word word)

/-- Appends one full-width EVM word as an RLP byte string. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def rlp_write_word (w : Nat) : SailM Unit := do
  (rlp_write_string_prefix WORD_BYTE_LENGTH 0x00#8)
  (scratch_push_word_be w WORD_BYTE_LENGTH)

/-- Appends one Ethereum address as an RLP byte string. -/
def rlp_write_addr (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  (rlp_write_string_prefix ADDRESS_BYTE_LENGTH (GetElem?.getElem! a 0))
  (scratch_push_address a)

def undefined_RlpEncoder (_ : Unit) : SailM RlpEncoder := do
  (pure { start := ← (undefined_range 0 ((2 ^i 32) - 1)),
          expected_len := ← (undefined_range 0 ((2 ^i 32) - 1)) })

/- Type quantifiers: expected_len : Nat, (source_valid_length expected_len) -/
def rlp_encoder_begin (expected_len : Nat) : SailM RlpEncoder := do
  (pure { start := ← (scratch_reserve expected_len),
          expected_len := expected_len })

/-- Finishes an exact-size construction and retains its encoded bytes. -/
def rlp_encoder_finish (encoder : RlpEncoder) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, encoded⟩⟩ ← do (scratch_finish encoder.start)
  assert (encoded.len == encoder.expected_len) "RLP encoder length"
  (pure ((⟨_, ⟨_, encoded⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))))

/-- Releases every byte appended by an encoder after its result is consumed. -/
def rlp_encoder_rewind (encoder : RlpEncoder) : SailM Unit := do
  (scratch_rewind encoder.start)

