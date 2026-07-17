import Evm.Flow
import Evm.Arith
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
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def RLP_SHORT_LENGTH_LIMIT : byte_length := (ByteQuantity 55)

def RLP_WORD_LENGTH_LIMIT : byte_length := WORD_BYTE_LENGTH

def RLP_ENCODED_WORD_LENGTH : byte_length := (ByteQuantity 33)

def RLP_ENCODED_ADDRESS_LENGTH : byte_length := (ByteQuantity 21)

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def minimal_protocol_quantity_bytes (n : protocol_quantity) : SailM ((List byte) × Nat) := do
  let n := (n).value
  let remaining : Nat := n
  let out : (List (BitVec 8)) := []
  let len : Nat := 0
  let (len, out, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (len, out, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, out, remaining) := loop_vars
      loop_vars ← do
        let (len, out, remaining) ← (( do
          if ((remaining != 0) : Bool)
          then
            (do
              let out : (List (BitVec 8)) := ((get_slice_int 8 (Int.tmod remaining 256) 0) :: out)
              let remaining ← (exact_quotient remaining 256)
              let len : Nat := (byte_index + 1)
              (pure (len, out, remaining)))
          else (pure (len, out, remaining)) ) : SailM (Nat × (List (BitVec 8)) × Nat) )
        (pure (len, out, remaining))
    (pure loop_vars) ) : SailM (Nat × (List (BitVec 8)) × Nat) )
  (pure (out, len))

def minimal_gas_bytes (app_0 : gas) : SailM ((List byte) × Nat) := do
  let .Gas n := app_0
  let remaining : gas := (Gas n)
  let out : (List (BitVec 8)) := []
  let len : Nat := 0
  let (len, out, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (len, out, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, out, remaining) := loop_vars
      loop_vars ← do
        let .Gas remaining_value := remaining
        let (len, out, remaining) ← (( do
          if ((remaining_value != 0) : Bool)
          then
            (do
              let out : (List (BitVec 8)) :=
                ((get_slice_int 8 (Int.tmod remaining_value 256) 0) :: out)
              let remaining ← (gas_quotient remaining ⟨256⟩)
              let len : Nat := (byte_index + 1)
              (pure (len, out, remaining)))
          else (pure (len, out, remaining)) ) : SailM (Nat × (List (BitVec 8)) × gas) )
        (pure (len, out, remaining))
    (pure loop_vars) ) : SailM (Nat × (List (BitVec 8)) × gas) )
  (pure (out, len))

def minimal_word_bytes (w : word) : ((List byte) × Nat) := Id.run do
  let remaining : (BitVec 256) := w
  let out : (List (BitVec 8)) := []
  let len : Nat := 0
  let (len, out, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (len, out, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, out, remaining) := loop_vars
      loop_vars :=
        let (len, out, remaining) : (Nat × (List (BitVec 8)) × (BitVec 256)) :=
          if ((word_nonzero remaining) : Bool)
          then
            (let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb remaining 7 0) :: out)
            let remaining : (BitVec 256) := (remaining >>> 8)
            let len : Nat := (byte_index + 1)
            (len, out, remaining))
          else (len, out, remaining)
        (len, out, remaining)
    (pure loop_vars) ) : Id (Nat × (List (BitVec 8)) × (BitVec 256)) )
  (pure (out, len))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 255 -/
def rlp_nat_length_byte (value : Nat) : byte :=
  (get_slice_int 8 value 0)

def rlp_byte_length_byte (value : byte_length) : SailM byte := do
  assert (byte_quantity_le value (ByteQuantity 255)) "sail/lib/rlp/rlp.sail:74.37-74.38"
  (pure (Sail.BitVec.extractLsb (← (byte_quantity_to_limb value)) 7 0))

def rlp_length_word (value : byte_length) : SailM word := do
  (word_of_byte_quantity value)

def rlp_string_prefix (len : byte_length) (first : byte) : SailM ((List byte) × byte_length) := do
  if (((byte_quantity_equal len BYTE_ONE) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure ([], BYTE_ZERO))
  else
    (do
      if ((byte_quantity_le len RLP_SHORT_LENGTH_LIMIT) : Bool)
      then (pure ([(0x80#8 + (← (rlp_byte_length_byte len)))], BYTE_ONE))
      else
        (do
          let (length_bytes, length_len) ← do
            (pure (minimal_word_bytes (← (rlp_length_word len))))
          (pure (((0xB7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (ByteQuantity
              (1 + length_len))))))

def rlp_list_prefix (len : byte_length) : SailM ((List byte) × byte_length) := do
  if ((byte_quantity_le len RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure ([(0xC0#8 + (← (rlp_byte_length_byte len)))], BYTE_ONE))
  else
    (do
      let (length_bytes, length_len) ← do (pure (minimal_word_bytes (← (rlp_length_word len))))
      (pure (((0xF7#8 + (rlp_nat_length_byte length_len)) :: length_bytes), (ByteQuantity
          (1 + length_len)))))

def rlp_minimal_word_len (w : word) : Nat := Id.run do
  let remaining : (BitVec 256) := w
  let len : Nat := 0
  let (len, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (len, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, remaining) := loop_vars
      loop_vars :=
        let (len, remaining) : (Nat × (BitVec 256)) :=
          if ((word_nonzero remaining) : Bool)
          then
            (let remaining : (BitVec 256) := (remaining >>> 8)
            let len : Nat := (byte_index + 1)
            (len, remaining))
          else (len, remaining)
        (len, remaining)
    (pure loop_vars) ) : Id (Nat × (BitVec 256)) )
  (pure len)

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_minimal_protocol_quantity_len (n : protocol_quantity) : SailM Nat := do
  let n := (n).value
  let remaining : Nat := n
  let len : Nat := 0
  let (len, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (len, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, remaining) := loop_vars
      loop_vars ← do
        let (len, remaining) ← (( do
          if ((remaining != 0) : Bool)
          then
            (do
              let remaining ← (exact_quotient remaining 256)
              let len : Nat := (byte_index + 1)
              (pure (len, remaining)))
          else (pure (len, remaining)) ) : SailM (Nat × Nat) )
        (pure (len, remaining))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure len)

def rlp_minimal_gas_len (app_0 : gas) : SailM Nat := do
  let .Gas n := app_0
  let remaining : gas := (Gas n)
  let len : Nat := 0
  let (len, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (len, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (len, remaining) := loop_vars
      loop_vars ← do
        let .Gas remaining_value := remaining
        let (len, remaining) ← (( do
          if ((remaining_value != 0) : Bool)
          then
            (do
              let remaining ← (gas_quotient remaining ⟨256⟩)
              let len : Nat := (byte_index + 1)
              (pure (len, remaining)))
          else (pure (len, remaining)) ) : SailM (Nat × gas) )
        (pure (len, remaining))
    (pure loop_vars) ) : SailM (Nat × gas) )
  (pure len)

def rlp_length_prefix_len (len : byte_length) : SailM byte_length := do
  if ((byte_quantity_le len RLP_SHORT_LENGTH_LIMIT) : Bool)
  then (pure BYTE_ONE)
  else (pure (ByteQuantity (1 + (rlp_minimal_word_len (← (rlp_length_word len))))))

def rlp_string_size (len : byte_length) (first : byte) : SailM byte_length := do
  if (((byte_quantity_equal len BYTE_ONE) && ((BitVec.access first 7) == 0#1)) : Bool)
  then (pure BYTE_ONE)
  else (byte_quantity_add (← (rlp_length_prefix_len len)) len)

def rlp_bytes_size (data : (List byte)) (len : byte_length) : SailM byte_length := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_string_size len first)

def rlp_slice_size (data : EvmByteSlice) : SailM byte_length := do
  (rlp_string_size data.len
    (← do
      if ((byte_quantity_equal data.len BYTE_ZERO) : Bool)
      then (pure 0x00#8)
      else (slice_byte data BYTE_ZERO)))

def rlp_uint_word_size (w : word) : SailM byte_length := do
  let len := (rlp_minimal_word_len w)
  (rlp_string_size (ByteQuantity len)
    (if ((len == 1) : Bool)
    then (Sail.BitVec.extractLsb w 7 0)
    else 0x00#8))

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_protocol_quantity_size (n : protocol_quantity) : SailM byte_length := do
  let n := (n).value
  let len ← do (rlp_minimal_protocol_quantity_len ⟨n⟩)
  (rlp_string_size (ByteQuantity len)
    (if ((len == 1) : Bool)
    then (get_slice_int 8 n 0)
    else 0x00#8))

def rlp_gas_size (value : gas) : SailM byte_length := do
  let .Gas n := value
  let len ← do (rlp_minimal_gas_len value)
  (rlp_string_size (ByteQuantity len)
    (if ((len == 1) : Bool)
    then (get_slice_int 8 n 0)
    else 0x00#8))

def rlp_word_size (_ : Unit) : byte_length :=
  RLP_ENCODED_WORD_LENGTH

def rlp_addr_size (_ : Unit) : byte_length :=
  RLP_ENCODED_ADDRESS_LENGTH

def rlp_list_size (content_len : byte_length) : SailM byte_length := do
  (byte_quantity_add (← (rlp_length_prefix_len content_len)) content_len)

def rlp_write_raw_bytes (data : (List byte)) (len : byte_length) : SailM Unit := do
  (scratch_push_bytes data len)

def rlp_write_raw_slice (data : EvmByteSlice) : SailM Unit := do
  (scratch_push_slice data)

def rlp_write_string_prefix (len : byte_length) (first : byte) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_string_prefix len first)
  (scratch_push_bytes encoded_prefix prefix_len)

def rlp_write_list_prefix (content_len : byte_length) : SailM Unit := do
  let (encoded_prefix, prefix_len) ← do (rlp_list_prefix content_len)
  (scratch_push_bytes encoded_prefix prefix_len)

def rlp_write_bytes (data : (List byte)) (len : byte_length) : SailM Unit := do
  let first : (BitVec 8) :=
    match data with
    | (b :: _) => b
    | [] => 0x00#8
  (rlp_write_string_prefix len first)
  (scratch_push_bytes data len)

def rlp_write_slice (data : EvmByteSlice) : SailM Unit := do
  (rlp_write_string_prefix data.len
    (← do
      if ((byte_quantity_equal data.len BYTE_ZERO) : Bool)
      then (pure 0x00#8)
      else (slice_byte data BYTE_ZERO)))
  (scratch_push_slice data)

def rlp_write_uint_word (w : word) : SailM Unit := do
  let (bytes, len) := (minimal_word_bytes w)
  (rlp_write_bytes bytes (ByteQuantity len))

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ (2 ^ 64 - 1) -/
def rlp_write_protocol_quantity (n : protocol_quantity) : SailM Unit := do
  let n := (n).value
  let (bytes, len) ← do (minimal_protocol_quantity_bytes ⟨n⟩)
  (rlp_write_bytes bytes (ByteQuantity len))

def rlp_write_gas (value : gas) : SailM Unit := do
  let (bytes, len) ← do (minimal_gas_bytes value)
  (rlp_write_bytes bytes (ByteQuantity len))

def rlp_write_word (w : word) : SailM Unit := do
  (rlp_write_bytes (word_to_bytes32 w) WORD_BYTE_LENGTH)

def rlp_write_addr (a : address) : SailM Unit := do
  (rlp_write_bytes (address_to_bytes a) ADDRESS_BYTE_LENGTH)

def rlp_finish (start : source_pointer) (expected_len : byte_length) : SailM EvmByteSlice := do
  let encoded ← do (scratch_finish start)
  assert (byte_quantity_equal encoded.len expected_len) "RLP forward writer length"
  (pure encoded)

def rlp_checked_add_quantity (a : source_pointer) (b : byte_length) : SailM source_pointer := do
  if ((byte_quantity_le a MAX_BYTE_QUANTITY) : Bool)
  then
    (do
      if ((byte_quantity_le b (← (byte_quantity_sub MAX_BYTE_QUANTITY a))) : Bool)
      then (byte_quantity_add a b)
      else sailThrow ((InvalidBlock RlpDecode)))
  else sailThrow ((InvalidBlock RlpDecode))

/-- Type quantifiers: k_ex161080_ : Nat, 0 ≤ k_ex161080_ -/
def rlp_checked_add_nat (a : source_pointer) (b : Nat) : SailM source_pointer := do
  (rlp_checked_add_quantity a (ByteQuantity b))

/-- Type quantifiers: k_ex161081_ : Nat, 0 ≤ k_ex161081_ ∧ k_ex161081_ ≤ 8 -/
def rlp_ref_be_length (source : EvmByteSlice) (start : source_pointer) (count : Nat) : SailM byte_length := do
  let value : (BitVec 64) := LIMB_ZERO
  let current : byte_quantity := start
  let (current, value) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 7
    let mut loop_vars := (current, value)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (current, value) := loop_vars
      loop_vars ← do
        let (current, value) ← (( do
          if ((byte_index <b count) : Bool)
          then
            (do
              let value ←
                (pure ((value <<< 8) ||| (Sail.BitVec.zeroExtend (← (slice_byte source current))
                      64)))
              let current ← (rlp_checked_add_quantity current BYTE_ONE)
              (pure (current, value)))
          else (pure (current, value)) ) : SailM (byte_quantity × (BitVec 64)) )
        (pure (current, value))
    (pure loop_vars) ) : SailM (byte_quantity × (BitVec 64)) )
  (pure (ByteQuantity (BitVec.toNatInt value)))

def rlp_bytes_equal_at (expected : (List byte)) (source : EvmByteSlice) (start : source_pointer) : SailM Bool := do
  let rest : (List (BitVec 8)) := expected
  let current : byte_quantity := start
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
              let equal ← (pure (equal && ((← (slice_byte source current)) == b)))
              let current ← (rlp_checked_add_quantity current BYTE_ONE)
              let rest : (List (BitVec 8)) := tail
              (pure (current, equal, rest)))
          | [] => (pure (current, equal, rest)) ) : SailM
          (byte_quantity × Bool × (List (BitVec 8))) )
        (pure (current, equal, rest))
    (pure loop_vars) ) : SailM (byte_quantity × Bool × (List (BitVec 8))) )
  match rest with
  | [] => (pure equal)
  | _ => (pure false)

def rlp_ref_hdr (b : EvmByteSlice) (pos : source_pointer) (stop : source_pointer) : SailM (Bool × source_pointer × byte_length) := do
  if ((! (byte_quantity_lt pos stop)) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let h ← do (pure (BitVec.toNatInt (← (slice_byte b pos))))
  if ((h <b 128) : Bool)
  then (pure (false, pos, BYTE_ONE))
  else
    (do
      if ((h <b 184) : Bool)
      then (pure (false, (← (rlp_checked_add_quantity pos BYTE_ONE)), (ByteQuantity (h -i 128))))
      else
        (do
          if ((h <b 192) : Bool)
          then
            (do
              let length_width : Nat := (h -i 183)
              let length_start ← do (rlp_checked_add_quantity pos BYTE_ONE)
              let content ← do (rlp_checked_add_nat length_start length_width)
              if ((byte_quantity_lt stop content) : Bool)
              then sailThrow ((InvalidBlock RlpDecode))
              else (pure ())
              (pure (false, content, (← (rlp_ref_be_length b length_start length_width)))))
          else
            (do
              if ((h <b 248) : Bool)
              then
                (pure (true, (← (rlp_checked_add_quantity pos BYTE_ONE)), (ByteQuantity (h -i 192))))
              else
                (do
                  let length_width : Nat := (h -i 247)
                  let length_start ← do (rlp_checked_add_quantity pos BYTE_ONE)
                  let content ← do (rlp_checked_add_nat length_start length_width)
                  if ((byte_quantity_lt stop content) : Bool)
                  then sailThrow ((InvalidBlock RlpDecode))
                  else (pure ())
                  (pure (true, content, (← (rlp_ref_be_length b length_start length_width))))))))

/-- Type quantifiers: k_ex161082_ : Bool -/
def rlp_field_ref (source : EvmByteSlice) (is_list : Bool) (full_off : source_pointer) (full_len : byte_length) (content_off : source_pointer) (content_len : byte_length) : RlpFieldRef :=
  { source := source,
    is_list := is_list,
    full_off := full_off,
    full_len := full_len,
    content_off := content_off,
    content_len := content_len }

def rlp_invalid_cursor (source : EvmByteSlice) : RlpCursor :=
  { source := source,
    current := BYTE_ZERO,
    stop := BYTE_ZERO,
    valid := false }

def rlp_node_cursor (node : EvmByteSlice) : SailM RlpCursor := do
  let n := node.len
  if ((byte_quantity_equal n BYTE_ZERO) : Bool)
  then (pure (rlp_invalid_cursor node))
  else
    (do
      let (is_list, content, content_len) ← do (rlp_ref_hdr node BYTE_ZERO n)
      let content_stop ← do (rlp_checked_add_quantity content content_len)
      if ((is_list && (byte_quantity_equal content_stop n)) : Bool)
      then
        (pure { source := node,
                current := content,
                stop := content_stop,
                valid := true })
      else (pure (rlp_invalid_cursor node)))

def rlp_ref_cursor (f : RlpFieldRef) : SailM RlpCursor := do
  if (f.is_list : Bool)
  then
    (do
      let stop ← do (rlp_checked_add_quantity f.content_off f.content_len)
      (pure { source := f.source,
              current := f.content_off,
              stop := stop,
              valid := true }))
  else sailThrow ((InvalidBlock RlpDecode))

def rlp_cursor_empty (cursor : RlpCursor) : Bool :=
  (cursor.valid && (byte_quantity_equal cursor.current cursor.stop))

def rlp_cursor_pop (cursor : RlpCursor) : SailM (RlpFieldRef × RlpCursor) := do
  let current : byte_quantity := cursor.current
  let stop : byte_quantity := cursor.stop
  if (((! cursor.valid) || (! (byte_quantity_lt current stop))) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let (is_list, content, content_len) ← do (rlp_ref_hdr cursor.source current stop)
  let next ← do (rlp_checked_add_quantity content content_len)
  if ((byte_quantity_lt stop next) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  if ((byte_quantity_le current next) : Bool)
  then
    (do
      let full_len ← (( do (byte_quantity_sub next current) ) : SailM byte_quantity )
      (pure ((rlp_field_ref cursor.source is_list current full_len content content_len), { source := cursor.source,
                                                                                           current := next,
                                                                                           stop := stop,
                                                                                           valid := true })))
  else sailThrow ((InvalidBlock RlpDecode))

def rlp_cursor_expect_end (cursor : RlpCursor) : SailM Unit := do
  if ((rlp_cursor_empty cursor) : Bool)
  then (pure ())
  else sailThrow ((InvalidBlock RlpDecode))

def rlp_single_ref (item : EvmByteSlice) : SailM RlpFieldRef := do
  let item_len := item.len
  if ((byte_quantity_equal item_len BYTE_ZERO) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else
    (do
      let (isl, cs, cl) ← do (rlp_ref_hdr item BYTE_ZERO item_len)
      let len ← do (rlp_checked_add_quantity cs cl)
      if ((byte_quantity_not_equal len item_len) : Bool)
      then sailThrow ((InvalidBlock RlpDecode))
      else (pure ())
      (pure (rlp_field_ref item isl BYTE_ZERO len cs cl)))

def rlp_ref_content (f : RlpFieldRef) : SailM EvmByteSlice := do
  (sub_slice f.source f.content_off f.content_len)

def rlp_ref_full (f : RlpFieldRef) : SailM EvmByteSlice := do
  (sub_slice f.source f.full_off f.full_len)

def rlp_ref_framing_canonical (f : RlpFieldRef) : SailM Bool := do
  let n := f.content_len
  let fl := f.full_len
  let fo := f.full_off
  let co := f.content_off
  if ((byte_quantity_equal fl BYTE_ZERO) : Bool)
  then (pure false)
  else
    (do
      if (f.is_list : Bool)
      then
        (do
          if ((byte_quantity_le n RLP_SHORT_LENGTH_LIMIT) : Bool)
          then
            (do
              let encoded_len ← do (byte_quantity_add n BYTE_ONE)
              (pure ((byte_quantity_equal fl encoded_len) && ((← (slice_byte f.source fo)) == (0xC0#8 + (← (rlp_byte_length_byte
                          n)))))))
          else
            (do
              let (len_bytes, length_width) ← do
                (pure (minimal_word_bytes (← (rlp_length_word n))))
              let encoded_len ← do (rlp_checked_add_quantity (ByteQuantity (1 + length_width)) n)
              (pure ((byte_quantity_equal fl encoded_len) && (((← (slice_byte f.source fo)) == (0xF7#8 + (rlp_nat_length_byte
                          length_width))) && (← (rlp_bytes_equal_at len_bytes f.source
                        (← (rlp_checked_add_quantity fo BYTE_ONE)))))))))
      else
        (do
          if ((byte_quantity_equal n BYTE_ZERO) : Bool)
          then
            (pure ((byte_quantity_equal fl BYTE_ONE) && ((← (slice_byte f.source fo)) == 0x80#8)))
          else
            (do
              let first ← do (slice_byte f.source co)
              if (((byte_quantity_equal n BYTE_ONE) && ((BitVec.access first 7) == 0#1)) : Bool)
              then
                (pure ((byte_quantity_equal fl BYTE_ONE) && ((← (slice_byte f.source fo)) == first)))
              else
                (do
                  if ((byte_quantity_le n RLP_SHORT_LENGTH_LIMIT) : Bool)
                  then
                    (do
                      let encoded_len ← do (byte_quantity_add n BYTE_ONE)
                      (pure ((byte_quantity_equal fl encoded_len) && ((← (slice_byte f.source fo)) == (0x80#8 + (← (rlp_byte_length_byte
                                  n)))))))
                  else
                    (do
                      let (len_bytes, length_width) ← do
                        (pure (minimal_word_bytes (← (rlp_length_word n))))
                      let encoded_len ← do
                        (rlp_checked_add_quantity (ByteQuantity (1 + length_width)) n)
                      (pure ((byte_quantity_equal fl encoded_len) && (((← (slice_byte f.source fo)) == (0xB7#8 + (rlp_nat_length_byte
                                  length_width))) && (← (rlp_bytes_equal_at len_bytes f.source
                                (← (rlp_checked_add_quantity fo BYTE_ONE))))))))))))

def rlp_ref_bytes_canonical (f : RlpFieldRef) : SailM Bool := do
  (pure ((! f.is_list) && (← (rlp_ref_framing_canonical f))))

def rlp_ref_uint_canonical (f : RlpFieldRef) : SailM Bool := do
  if ((! (← (rlp_ref_bytes_canonical f))) : Bool)
  then (pure false)
  else
    (pure ((byte_quantity_equal f.content_len BYTE_ZERO) || ((← (slice_byte f.source f.content_off)) != 0x00#8)))

def rlp_ref_fixed_bytes (f : RlpFieldRef) (len : byte_length) : SailM EvmByteSlice := do
  if (((← (rlp_ref_bytes_canonical f)) && (byte_quantity_equal f.content_len len)) : Bool)
  then (rlp_ref_content f)
  else sailThrow ((InvalidBlock RlpDecode))

def rlp_ref_word (f : RlpFieldRef) : SailM word := do
  let n := f.content_len
  if ((f.is_list || (byte_quantity_lt RLP_WORD_LENGTH_LIMIT n)) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (slice_load_n f.source f.content_off n)

def rlp_ref_uint_word (f : RlpFieldRef) : SailM word := do
  if ((← (rlp_ref_uint_canonical f)) : Bool)
  then (rlp_ref_word f)
  else sailThrow ((InvalidBlock RlpDecode))

def rlp_ref_uint (f : RlpFieldRef) : SailM protocol_quantity := do
  let semanticResult ← do
    let value ← do (pure (BitVec.toNatInt (← (rlp_ref_uint_word f))))
    if ((value ≤b ((2 ^i 64) -i 1)) : Bool)
    then (pure value)
    else sailThrow ((InvalidBlock RlpDecode))
  pure (⟨semanticResult⟩)

