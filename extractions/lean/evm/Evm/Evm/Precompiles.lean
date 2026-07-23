import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice
import Evm.Host.Output
import Evm.Host.Kernel.Environment
import Evm.Evm.Gas

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

/-! # Precompiled contracts

EVM precompile semantics (YP Appendix E and the precompile EIPs). Sail
selects the active address, validates the Ethereum input encoding, and
returns the EVM-visible output; the host functions called here are only
raw zkVM cryptographic accelerators.

## Constants

The fixed offsets and lengths describe the canonical byte layouts accepted
by each precompile; the field modulus and blob element count validate their
respective algebraic inputs. -/

def ACCELERATOR_INPUT_MAX : Nat := 2097152

def FIELD_ELEMENTS_PER_BLOB : word :=
  ⟨((U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000001000#256))).value⟩

def BLS_MODULUS : word :=
  ⟨((U256 (BitVec.toNatInt 0x73EDA753299D7D483339D80809A1D80553BDA402FFFE5BFEFFFFFFFF00000001#256))).value⟩

def BLAKE2F_INPUT_LENGTH : Nat := 213

def BLAKE2F_FINAL_BLOCK_OFFSET : Nat := 212

def BLAKE2F_OUTPUT_LENGTH : Nat := DOUBLE_WORD_BYTE_LENGTH

def KZG_INPUT_LENGTH : Nat := 192

def KZG_COMMITMENT_OFFSET : Nat := 96

def KZG_COMMITMENT_LENGTH : Nat := 48

def BLS_FIELD_PADDING_LENGTH : Nat := 16

def BLS_PADDED_FIELD_LENGTH : Nat := DOUBLE_WORD_BYTE_LENGTH

def BLS_G1_POINT_LENGTH : Nat := 128

def BLS_G2_POINT_LENGTH : Nat := 256

def BLS_G2_FINAL_FIELD_OFFSET : Nat := 192

def BLS_G1_ADD_INPUT_LENGTH : Nat := 256

def BLS_G1_MSM_ITEM_LENGTH : Nat := 160

def BLS_G2_ADD_INPUT_LENGTH : Nat := 512

def BLS_G2_MSM_ITEM_LENGTH : Nat := 288

def BLS_PAIRING_ITEM_LENGTH : Nat := 384

def P256_INPUT_LENGTH : Nat := 160

def BN254_PAIRING_ITEM_LENGTH : Nat := 192

def PRECOMPILE_WORD_LENGTH : Nat := WORD_BYTE_LENGTH

def PRECOMPILE_DOUBLE_WORD_LENGTH : Nat := DOUBLE_WORD_BYTE_LENGTH

def PRECOMPILE_WORD_OFFSET : Nat := 32

def PRECOMPILE_DOUBLE_WORD_OFFSET : Nat := 64

def ECRECOVER_S_OFFSET : source_pointer := 96

def TWO_COMPONENTS : Nat := 2

def BLS_G2_POINT_OFFSET : Nat := 128

def PRECOMPILE_ADDRESS_1 : address :=
  #v[0x01#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_2 : address :=
  #v[0x02#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_3 : address :=
  #v[0x03#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_4 : address :=
  #v[0x04#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_5 : address :=
  #v[0x05#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_6 : address :=
  #v[0x06#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_7 : address :=
  #v[0x07#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_8 : address :=
  #v[0x08#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_9 : address :=
  #v[0x09#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_10 : address :=
  #v[0x0A#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_11 : address :=
  #v[0x0B#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_12 : address :=
  #v[0x0C#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_13 : address :=
  #v[0x0D#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_14 : address :=
  #v[0x0E#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_15 : address :=
  #v[0x0F#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_16 : address :=
  #v[0x10#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_17 : address :=
  #v[0x11#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

def PRECOMPILE_ADDRESS_256 : address :=
  #v[0x00#8, 0x01#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]

/-- A successful result carrying `output`. -/
/- Type quantifiers: k_ex408598_ : Nat, k_ex408597_ : Nat, 0 ≤ k_ex408597_ ∧ 0 ≤ k_ex408598_ -/
def precompile_success (output : EvmByteSlice) : PrecompileResult :=
  let output := ((output).2).2
  { success := true,
    output := ⟨_, ⟨_, output⟩⟩ }

/-- The failed result (empty output; the call reports failure). -/
def precompile_failure (_ : Unit) : PrecompileResult :=
  { success := false,
    output := ⟨_, ⟨_, EMPTY_SLICE⟩⟩ }

/- Type quantifiers: k_ex408602_ : Bool, output_len : Nat, source_valid_length(output_len) -/
def accelerator_result (success : Bool) (output_len : Nat) : PrecompileResult :=
  if (success : Bool)
  then
    (precompile_success
      (⟨_, ⟨_, (((output_buffer_slice output_len)).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else (precompile_failure ())

/-- `IDENTITY` (0x04): the input, copied through the output buffer. -/
/- Type quantifiers: k_ex408608_ : Nat, k_ex408607_ : Nat, 0 ≤ k_ex408607_ ∧ 0 ≤ k_ex408608_ -/
def copied_result (data : EvmByteSlice) : SailM PrecompileResult := do
  let data := ((data).2).2
  let ⟨_, ⟨_, output⟩⟩ ← do (freeze_output ⟨_, ⟨_, data⟩⟩)
  if ((output.len == data.len) : Bool)
  then (pure (precompile_success ⟨_, ⟨_, output⟩⟩))
  else (pure (precompile_failure ()))

/-- A 32-byte `0`/`1` result word (pairing checks). -/
/- Type quantifiers: k_ex408609_ : Bool -/
def boolean_result (value : Bool) : SailM PrecompileResult := do
  (pure (precompile_success
      (← (output_buffer_word
          ⟨(if (value : Bool)
          then (WORD_ONE).value
          else (WORD_ZERO).value)⟩))))

/-- Whether address `n` is an active precompile at the current fork:
1–4 always; 5–8 from Byzantium; 9 from Istanbul; 10 from Cancun
(EIP-4844); 11–17 from Prague (EIP-2537); 0x100 from Osaka
(EIP-7951). -/
/- Type quantifiers: n : Nat, 1 ≤ n ∧ n ≤ 256 -/
def precompile_active_at_fork (n : precompile_id) : SailM Bool := do
  let n := (n).value
  let base := (n ≤b 4)
  let byzantium ← do
    (pure ((5 ≤b n) && ((n ≤b 8) && (fork_gteq (← readReg k_fork) Byzantium))))
  let istanbul ← do (pure ((n == 9) && (fork_gteq (← readReg k_fork) Istanbul)))
  let kzg ← do (pure ((n == 10) && (fork_gteq (← readReg k_fork) Cancun)))
  let bls ← do (pure ((11 ≤b n) && ((n ≤b 17) && (fork_gteq (← readReg k_fork) Prague))))
  let p256 ← do (pure ((n == 256) && (fork_gteq (← readReg k_fork) Osaka)))
  (pure (base || (byzantium || (istanbul || (kzg || (bls || p256))))))

/-- Returns the active precompile number represented by `target`, or zero
when it is an ordinary address at the current fork. -/
def precompile_number (bytes : address) : SailM precompile_selector := do
  let publicResult ← do
    let selector : (BitVec 16) := ((GetElem?.getElem! bytes 1) +++ (GetElem?.getElem! bytes 0))
    let candidate : Nat :=
      match selector with
      | 0x0001 =>
        (if ((bytes == PRECOMPILE_ADDRESS_1) : Bool)
        then 1
        else 0)
      | 0x0002 =>
        (if ((bytes == PRECOMPILE_ADDRESS_2) : Bool)
        then 2
        else 0)
      | 0x0003 =>
        (if ((bytes == PRECOMPILE_ADDRESS_3) : Bool)
        then 3
        else 0)
      | 0x0004 =>
        (if ((bytes == PRECOMPILE_ADDRESS_4) : Bool)
        then 4
        else 0)
      | 0x0005 =>
        (if ((bytes == PRECOMPILE_ADDRESS_5) : Bool)
        then 5
        else 0)
      | 0x0006 =>
        (if ((bytes == PRECOMPILE_ADDRESS_6) : Bool)
        then 6
        else 0)
      | 0x0007 =>
        (if ((bytes == PRECOMPILE_ADDRESS_7) : Bool)
        then 7
        else 0)
      | 0x0008 =>
        (if ((bytes == PRECOMPILE_ADDRESS_8) : Bool)
        then 8
        else 0)
      | 0x0009 =>
        (if ((bytes == PRECOMPILE_ADDRESS_9) : Bool)
        then 9
        else 0)
      | 0x000A =>
        (if ((bytes == PRECOMPILE_ADDRESS_10) : Bool)
        then 10
        else 0)
      | 0x000B =>
        (if ((bytes == PRECOMPILE_ADDRESS_11) : Bool)
        then 11
        else 0)
      | 0x000C =>
        (if ((bytes == PRECOMPILE_ADDRESS_12) : Bool)
        then 12
        else 0)
      | 0x000D =>
        (if ((bytes == PRECOMPILE_ADDRESS_13) : Bool)
        then 13
        else 0)
      | 0x000E =>
        (if ((bytes == PRECOMPILE_ADDRESS_14) : Bool)
        then 14
        else 0)
      | 0x000F =>
        (if ((bytes == PRECOMPILE_ADDRESS_15) : Bool)
        then 15
        else 0)
      | 0x0010 =>
        (if ((bytes == PRECOMPILE_ADDRESS_16) : Bool)
        then 16
        else 0)
      | 0x0011 =>
        (if ((bytes == PRECOMPILE_ADDRESS_17) : Bool)
        then 17
        else 0)
      | 0x0100 =>
        (if ((bytes == PRECOMPILE_ADDRESS_256) : Bool)
        then 256
        else 0)
      | _ => 0
    if ((candidate != 0) : Bool)
    then
      (do
        let id : Nat := candidate
        if ((← (precompile_active_at_fork ⟨id⟩)) : Bool)
        then (pure id)
        else (pure 0))
    else (pure 0)
  pure (⟨publicResult⟩)

/-- `ECRECOVER` (0x01): recovers the signer address; any invalid input
yields a successful call with empty output. -/
/- Type quantifiers: k_ex408614_ : Nat, k_ex408613_ : Nat, 0 ≤ k_ex408613_ ∧ 0 ≤ k_ex408614_ -/
def run_ecrecover (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let v ← do
    (do
        let publicResult ← (slice_load ⟨_, ⟨_, input⟩⟩ PRECOMPILE_WORD_OFFSET)
        pure ((publicResult).value))
  let valid_v := ((v == ((U256 27)).value) || ((v == ((U256 28)).value) : Bool))
  if (valid_v : Bool)
  then
    (do
      let parity : Nat :=
        if ((v == ((U256 27)).value) : Bool)
        then 0
        else 1
      let (recovered, address) ← do
        (ecrecover_addr (word_to_hash ⟨((← (slice_load ⟨_, ⟨_, input⟩⟩ 0))).value⟩)
          ⟨parity⟩
          ⟨((← (slice_load ⟨_, ⟨_, input⟩⟩ PRECOMPILE_DOUBLE_WORD_OFFSET))).value⟩
          ⟨((← (slice_load ⟨_, ⟨_, input⟩⟩ ECRECOVER_S_OFFSET))).value⟩)
      if (recovered : Bool)
      then
        (pure (precompile_success (← (output_buffer_word ⟨((address_to_word address)).value⟩))))
      else (pure (precompile_success ⟨_, ⟨_, EMPTY_SLICE⟩⟩)))
  else (pure (precompile_success ⟨_, ⟨_, EMPTY_SLICE⟩⟩))

/-- `SHA256` (0x02). -/
/- Type quantifiers: k_ex408618_ : Nat, k_ex408617_ : Nat, 0 ≤ k_ex408617_ ∧ 0 ≤ k_ex408618_ -/
def run_sha256 (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  (pure (precompile_success
      (← (output_buffer_word
          ⟨((hash_to_word (← (sha256_slice ⟨_, ⟨_, input⟩⟩)))).value⟩))))

/-- `RIPEMD160` (0x03): 20-byte digest, left-padded to 32. -/
/- Type quantifiers: k_ex408622_ : Nat, k_ex408621_ : Nat, 0 ≤ k_ex408621_ ∧ 0 ≤ k_ex408622_ -/
def run_ripemd160 (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  (pure (accelerator_result (← (accelerator_ripemd160 ⟨_, ⟨_, input⟩⟩))
      PRECOMPILE_WORD_LENGTH))

/-- `MODEXP` (0x05, EIP-198): arbitrary-precision modular
exponentiation; a zero-length modulus yields empty output, and inputs
beyond the accelerator bound fail the call. -/
/- Type quantifiers: k_ex408626_ : Nat, k_ex408625_ : Nat, 0 ≤ k_ex408625_ ∧ 0 ≤ k_ex408626_ -/
def run_modexp (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let base_len ← (( do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 0 32)
        pure ((publicResult).value)) ) : SailM Nat )
  let exponent_len ← (( do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 32 32)
        pure ((publicResult).value)) ) : SailM Nat )
  let modulus_len ← (( do
    (do
        let publicResult ← (pc_word ⟨_, ⟨_, input⟩⟩ 64 32)
        pure ((publicResult).value)) ) : SailM Nat )
  if ((modulus_len == 0) : Bool)
  then (pure (precompile_success ⟨_, ⟨_, EMPTY_SLICE⟩⟩))
  else
    (do
      if (((ACCELERATOR_INPUT_MAX <b base_len) || ((ACCELERATOR_INPUT_MAX <b exponent_len) || (ACCELERATOR_INPUT_MAX <b modulus_len))) : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let bounded_base : Nat := base_len
          let bounded_exponent : Nat := exponent_len
          let bounded_modulus : Nat := modulus_len
          let input_end : Nat := (((96 + bounded_base) + bounded_exponent) + bounded_modulus)
          if ((ACCELERATOR_INPUT_MAX <b input_end) : Bool)
          then (pure (precompile_failure ()))
          else
            (pure (accelerator_result
                (← (accelerator_modexp ⟨_, ⟨_, input⟩⟩ bounded_base bounded_exponent
                    bounded_modulus)) bounded_modulus))))

/-- Decodes an accelerator pairing result: bit 1 is validity of the
input, bit 0 the pairing outcome. -/
def pairing_result (result : (BitVec 2)) : SailM PrecompileResult := do
  if (((BitVec.access result 1) == 0#1) : Bool)
  then (pure (precompile_failure ()))
  else (boolean_result ((BitVec.access result 0) == 1#1))

/-- `BLAKE2F` (0x09, EIP-152): the compression function; the input must
be exactly 213 bytes with a 0/1 final-block flag. -/
/- Type quantifiers: k_ex408631_ : Nat, k_ex408630_ : Nat, 0 ≤ k_ex408630_ ∧ 0 ≤ k_ex408631_ -/
def run_blake2f (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let final_byte ← do (slice_byte ⟨_, ⟨_, input⟩⟩ BLAKE2F_FINAL_BLOCK_OFFSET)
  if (((input.len != BLAKE2F_INPUT_LENGTH) || ((final_byte != 0x00#8) && (final_byte != 0x01#8))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let final_block : Nat :=
        if ((final_byte == 0x00#8) : Bool)
        then 0
        else 1
      (pure (accelerator_result
          (← (accelerator_blake2f ⟨_, ⟨_, input⟩⟩
              ⟨((← (pc_blake2_rounds ⟨_, ⟨_, input⟩⟩))).value⟩ ⟨final_block⟩))
          BLAKE2F_OUTPUT_LENGTH)))

/- Type quantifiers: k_base : Nat, k_len : Nat, source_valid_range(k_base, k_len) ∧ k_len = 192 -/
def kzg_versioned_hash_matches (input : (EvmByteSliceFields k_base k_len)) : SailM Bool := do
  let commitment_hash ← do
    (sha256_slice ⟨_, ⟨_, (sub_slice input KZG_COMMITMENT_OFFSET KZG_COMMITMENT_LENGTH)⟩⟩)
  let expected := commitment_hash
  let expected : (Vector (BitVec 8) 32) := (vectorUpdate expected 31 0x01#8)
  (pure ((word_to_hash ⟨((← (slice_load ⟨_, ⟨_, input⟩⟩ 0))).value⟩) == expected))

/-- `POINT_EVALUATION` (0x0a, EIP-4844): verifies a KZG proof; success
returns the field-elements-per-blob and BLS modulus constants. -/
/- Type quantifiers: k_ex408647_ : Nat, k_ex408646_ : Nat, 0 ≤ k_ex408646_ ∧ 0 ≤ k_ex408647_ -/
def run_kzg_point_evaluation (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  if ((input.len != KZG_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (kzg_versioned_hash_matches input))) : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          if ((← (accelerator_kzg_point_evaluation ⟨_, ⟨_, input⟩⟩)) : Bool)
          then
            (pure (precompile_success
                (← (output_buffer_words ⟨(FIELD_ELEMENTS_PER_BLOB).value⟩
                    ⟨(BLS_MODULUS).value⟩))))
          else (pure (precompile_failure ()))))

/- Type quantifiers: k_ex408663_ : Nat, k_ex408662_ : Nat, base : Nat, stride : Nat, count : Nat, source_valid_range(base, 64)
  ∧ source_valid_length(stride) ∧ source_valid_length(count), 0 ≤ k_ex408662_ ∧
  0 ≤ k_ex408663_ -/
def bls_g1_padding (input : EvmByteSlice) (base : Nat) (stride : Nat) (count : Nat) : SailM Bool := do
  let input := ((input).2).2
  (pure ((← (slice_strided_zero ⟨_, ⟨_, input⟩⟩ base stride BLS_FIELD_PADDING_LENGTH count)) && (← (slice_strided_zero
          ⟨_, ⟨_, input⟩⟩ (base + BLS_PADDED_FIELD_LENGTH) stride BLS_FIELD_PADDING_LENGTH
          count))))

/- Type quantifiers: k_ex408687_ : Nat, k_ex408686_ : Nat, base : Nat, stride : Nat, count : Nat, source_valid_range(base, 192)
  ∧ source_valid_length(stride) ∧ source_valid_length(count), 0 ≤ k_ex408686_ ∧
  0 ≤ k_ex408687_ -/
def bls_g2_padding (input : EvmByteSlice) (base : Nat) (stride : Nat) (count : Nat) : SailM Bool := do
  let input := ((input).2).2
  (pure ((← (slice_strided_zero ⟨_, ⟨_, input⟩⟩ base stride BLS_FIELD_PADDING_LENGTH count)) && ((← (slice_strided_zero
            ⟨_, ⟨_, input⟩⟩ (base + BLS_PADDED_FIELD_LENGTH) stride BLS_FIELD_PADDING_LENGTH
            count)) && ((← (slice_strided_zero ⟨_, ⟨_, input⟩⟩
              (base + BLS_G1_POINT_LENGTH) stride BLS_FIELD_PADDING_LENGTH count)) && (← (slice_strided_zero
              ⟨_, ⟨_, input⟩⟩ (base + BLS_G2_FINAL_FIELD_OFFSET) stride
              BLS_FIELD_PADDING_LENGTH count))))))

/-- `BLS12_G1ADD` (0x0b, EIP-2537). -/
/- Type quantifiers: k_ex408699_ : Nat, k_ex408698_ : Nat, 0 ≤ k_ex408698_ ∧ 0 ≤ k_ex408699_ -/
def run_bls_g1_add (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  if ((input.len != BLS_G1_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g1_padding ⟨_, ⟨_, input⟩⟩ 0 BLS_G1_POINT_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_g1_add ⟨_, ⟨_, input⟩⟩))
            BLS_G1_POINT_LENGTH)))

/-- `BLS12_G1MSM` (0x0c, EIP-2537): input is `k` 160-byte pairs. -/
/- Type quantifiers: k_ex408703_ : Nat, k_ex408702_ : Nat, 0 ≤ k_ex408702_ ∧ 0 ≤ k_ex408703_ -/
def run_bls_g1_msm (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let length := input.len
  let item_length := BLS_G1_MSM_ITEM_LENGTH
  let pairs := (Int.ediv length item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g1_padding ⟨_, ⟨_, input⟩⟩ 0 BLS_G1_MSM_ITEM_LENGTH pairs))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_g1_msm ⟨_, ⟨_, input⟩⟩))
            BLS_G1_POINT_LENGTH)))

/-- `BLS12_G2ADD` (0x0d, EIP-2537). -/
/- Type quantifiers: k_ex408707_ : Nat, k_ex408706_ : Nat, 0 ≤ k_ex408706_ ∧ 0 ≤ k_ex408707_ -/
def run_bls_g2_add (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  if ((input.len != BLS_G2_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g2_padding ⟨_, ⟨_, input⟩⟩ 0 BLS_G2_POINT_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_g2_add ⟨_, ⟨_, input⟩⟩))
            BLS_G2_POINT_LENGTH)))

/-- `BLS12_G2MSM` (0x0e, EIP-2537): input is `k` 288-byte pairs. -/
/- Type quantifiers: k_ex408711_ : Nat, k_ex408710_ : Nat, 0 ≤ k_ex408710_ ∧ 0 ≤ k_ex408711_ -/
def run_bls_g2_msm (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let length := input.len
  let item_length := BLS_G2_MSM_ITEM_LENGTH
  let pairs := (Int.ediv length item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g2_padding ⟨_, ⟨_, input⟩⟩ 0 BLS_G2_MSM_ITEM_LENGTH pairs))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_g2_msm ⟨_, ⟨_, input⟩⟩))
            BLS_G2_POINT_LENGTH)))

/-- `BLS12_PAIRING_CHECK` (0x0f, EIP-2537): input is `k` 384-byte
G1×G2 pairs. -/
/- Type quantifiers: k_ex408715_ : Nat, k_ex408714_ : Nat, 0 ≤ k_ex408714_ ∧ 0 ≤ k_ex408715_ -/
def run_bls_pairing (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let length := input.len
  let item_length := BLS_PAIRING_ITEM_LENGTH
  let pairs := (Int.ediv length item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let pair_count : Nat := pairs
      if (((← (bls_g1_padding ⟨_, ⟨_, input⟩⟩ 0 BLS_PAIRING_ITEM_LENGTH pair_count)) && (← (bls_g2_padding
               ⟨_, ⟨_, input⟩⟩ BLS_G2_POINT_OFFSET BLS_PAIRING_ITEM_LENGTH pair_count))) : Bool)
      then (pairing_result (← (accelerator_bls_pairing ⟨_, ⟨_, input⟩⟩)))
      else (pure (precompile_failure ())))

/-- `BLS12_MAP_FP_TO_G1` (0x10, EIP-2537). -/
/- Type quantifiers: k_ex408719_ : Nat, k_ex408718_ : Nat, 0 ≤ k_ex408718_ ∧ 0 ≤ k_ex408719_ -/
def run_bls_map_fp_to_g1 (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  if ((input.len != BLS_PADDED_FIELD_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (slice_strided_zero ⟨_, ⟨_, input⟩⟩ 0 BLS_PADDED_FIELD_LENGTH
               BLS_FIELD_PADDING_LENGTH 1))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_map_fp_to_g1 ⟨_, ⟨_, input⟩⟩))
            BLS_G1_POINT_LENGTH)))

/-- `BLS12_MAP_FP2_TO_G2` (0x11, EIP-2537). -/
/- Type quantifiers: k_ex408723_ : Nat, k_ex408722_ : Nat, 0 ≤ k_ex408722_ ∧ 0 ≤ k_ex408723_ -/
def run_bls_map_fp2_to_g2 (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  if ((input.len != BLS_G1_POINT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (slice_strided_zero ⟨_, ⟨_, input⟩⟩ 0 BLS_PADDED_FIELD_LENGTH
               BLS_FIELD_PADDING_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_map_fp2_to_g2 ⟨_, ⟨_, input⟩⟩))
            BLS_G2_POINT_LENGTH)))

/-- `P256VERIFY` (0x100, EIP-7951): every malformed or invalid signature
is a successful call with empty output; only a valid signature
returns the word one. -/
/- Type quantifiers: k_ex408727_ : Nat, k_ex408726_ : Nat, 0 ≤ k_ex408726_ ∧ 0 ≤ k_ex408727_ -/
def run_p256_verify (input : EvmByteSlice) : SailM PrecompileResult := do
  let input := ((input).2).2
  let verified ← do
    if ((input.len == P256_INPUT_LENGTH) : Bool)
    then (accelerator_p256_verify ⟨_, ⟨_, input⟩⟩)
    else (pure false)
  if (verified : Bool)
  then (pure (precompile_success (← (output_buffer_word ⟨(WORD_ONE).value⟩))))
  else (pure (precompile_success ⟨_, ⟨_, EMPTY_SLICE⟩⟩))

/-- The precompile dispatch: address to implementation. Gas has already
been charged by the caller ([precompile_gas][]). -/
/- Type quantifiers: k_ex408732_ : Nat, k_ex408731_ : Nat, k_ex408728_ : Nat, 1 ≤ k_ex408728_ ∧
  k_ex408728_ ≤ 256, 0 ≤ k_ex408731_ ∧ 0 ≤ k_ex408732_ -/
def run_precompile_slice (num : precompile_id) (input : EvmByteSlice) : SailM PrecompileResult := do
  let num := (num).value
  let input := ((input).2).2
  match num with
  | 1 => (run_ecrecover ⟨_, ⟨_, input⟩⟩)
  | 2 => (run_sha256 ⟨_, ⟨_, input⟩⟩)
  | 3 => (run_ripemd160 ⟨_, ⟨_, input⟩⟩)
  | 4 => (copied_result ⟨_, ⟨_, input⟩⟩)
  | 5 => (run_modexp ⟨_, ⟨_, input⟩⟩)
  | 6 =>
    (pure (accelerator_result (← (accelerator_bn254_add ⟨_, ⟨_, input⟩⟩))
        PRECOMPILE_DOUBLE_WORD_LENGTH))
  | 7 =>
    (pure (accelerator_result (← (accelerator_bn254_mul ⟨_, ⟨_, input⟩⟩))
        PRECOMPILE_DOUBLE_WORD_LENGTH))
  | 8 =>
    (do
      let input_length := input.len
      let item_length := BN254_PAIRING_ITEM_LENGTH
      if ((input_length == ((Int.ediv input_length item_length) *i item_length)) : Bool)
      then (pairing_result (← (accelerator_bn254_pairing ⟨_, ⟨_, input⟩⟩)))
      else (pure (precompile_failure ())))
  | 9 => (run_blake2f ⟨_, ⟨_, input⟩⟩)
  | 10 => (run_kzg_point_evaluation ⟨_, ⟨_, input⟩⟩)
  | 11 => (run_bls_g1_add ⟨_, ⟨_, input⟩⟩)
  | 12 => (run_bls_g1_msm ⟨_, ⟨_, input⟩⟩)
  | 13 => (run_bls_g2_add ⟨_, ⟨_, input⟩⟩)
  | 14 => (run_bls_g2_msm ⟨_, ⟨_, input⟩⟩)
  | 15 => (run_bls_pairing ⟨_, ⟨_, input⟩⟩)
  | 16 => (run_bls_map_fp_to_g1 ⟨_, ⟨_, input⟩⟩)
  | 17 => (run_bls_map_fp2_to_g2 ⟨_, ⟨_, input⟩⟩)
  | 256 => (run_p256_verify ⟨_, ⟨_, input⟩⟩)
  | _ => (pure (precompile_failure ()))

