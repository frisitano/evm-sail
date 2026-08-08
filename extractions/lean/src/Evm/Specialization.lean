import Sail
import Evm.Defs
import Evm.HostAxioms

open Sail

namespace Evm

open Defs

private abbrev BaseSailM := Evm.Defs.SailM

private def liftBase (action : BaseSailM α) : SailM α :=
  StateT.lift action

@[simp_sail]
def sailTryCatch (action : SailM α) (handler : exception → SailM α) : SailM α :=
  fun state =>
    PreSail.sailTryCatch (action.run state) fun error =>
      (handler error).run state

@[simp_sail]
def sailThrow (error : exception) : SailM α :=
  liftBase (PreSail.sailThrow error)

abbrev undefined_unit (_ : Unit) : SailM Unit :=
  liftBase (PreSail.undefined_unit ())

abbrev undefined_bit (_ : Unit) : SailM (BitVec 1) :=
  liftBase (PreSail.undefined_bit ())

abbrev undefined_bool (_ : Unit) : SailM Bool :=
  liftBase (PreSail.undefined_bool ())

abbrev undefined_int (_ : Unit) : SailM Int :=
  liftBase (PreSail.undefined_int ())

abbrev undefined_range (low high : Int) : SailM Int :=
  liftBase (PreSail.undefined_range low high)

abbrev undefined_nat (_ : Unit) : SailM Nat :=
  liftBase (PreSail.undefined_nat ())

abbrev undefined_string (_ : Unit) : SailM String :=
  liftBase (PreSail.undefined_string ())

abbrev undefined_bitvector (n : Nat) : SailM (BitVec n) :=
  liftBase (PreSail.undefined_bitvector n)

abbrev undefined_vector (n : Nat) (a : α) : SailM (Vector α n) :=
  liftBase (PreSail.undefined_vector n a)

abbrev internal_pick {α : Type} (values : List α) : SailM α :=
  liftBase (PreSail.internal_pick values)

abbrev writeReg (reg : Register) (value : RegisterType reg) : SailM PUnit :=
  liftBase (PreSail.writeReg reg value)

abbrev readReg (reg : Register) : SailM (RegisterType reg) :=
  liftBase (PreSail.readReg reg)

abbrev RegisterRef := @PreSail.RegisterRef Register RegisterType

abbrev readRegRef (regRef : RegisterRef α) : SailM α :=
  liftBase (PreSail.readRegRef regRef)

abbrev writeRegRef (regRef : RegisterRef α) (value : α) : SailM Unit :=
  liftBase (PreSail.writeRegRef regRef value)

abbrev reg_deref (regRef : RegisterRef α) : SailM α :=
  readRegRef regRef

abbrev assert (condition : Bool) (message : String) : SailM Unit :=
  liftBase (PreSail.assert condition message)

namespace ConcurrencyInterfaceV1

open Sail.ConcurrencyInterfaceV1

abbrev sail_mem_write [Arch]
    (request : Mem_write_request n vasize (BitVec pa_size) ts arch) :
    SailM (Result (Option Bool) Arch.abort) :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_mem_write request)

abbrev write_ram
    (addressSize dataSize : Nat) (hexRam address : BitVec addressSize)
    (value : BitVec (8 * dataSize)) : SailM Unit :=
  liftBase (PreSail.write_ram addressSize dataSize hexRam address value)

abbrev sail_mem_read [Arch]
    (request : Mem_read_request n vasize (BitVec pa_size) ts arch) :
    SailM (Result ((BitVec (8 * n)) × Option Bool) Arch.abort) :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_mem_read request)

abbrev read_ram
    (addressSize dataSize : Nat) (hexRam address : BitVec addressSize) :
    SailM (BitVec (8 * dataSize)) :=
  liftBase (PreSail.read_ram addressSize dataSize hexRam address)

abbrev sail_barrier (value : α) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_barrier value)

abbrev sail_cache_op [Arch] (operation : Arch.cache_op) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_cache_op operation)

abbrev sail_tlbi [Arch] (operation : Arch.tlb_op) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_tlbi operation)

abbrev sail_translation_start [Arch] (value : Arch.trans_start) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_translation_start value)

abbrev sail_translation_end [Arch] (value : Arch.trans_end) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_translation_end value)

abbrev sail_take_exception [Arch] (fault : Arch.fault) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_take_exception fault)

abbrev sail_return_exception [Arch] (address : Arch.pa) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV1.sail_return_exception address)

end ConcurrencyInterfaceV1

namespace ConcurrencyInterfaceV2

open Sail.ConcurrencyInterfaceV2

abbrev sail_mem_read [Arch]
    (request : Mem_request n nt Arch.addr_size Arch.addr_space Arch.mem_acc) :
    SailM (Result ((Vector (BitVec 8) n) × Vector Bool nt) Arch.abort) :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_mem_read request)

def sail_mem_write [Arch]
    (request : Mem_request n nt Arch.addr_size Arch.addr_space Arch.mem_acc)
    (valueBytes : Vector (BitVec 8) n) (tags : Vector Bool nt) :
    SailM (Result (Option Bool) Arch.abort) :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_mem_write request valueBytes tags)

abbrev sail_sys_reg_read [Arch]
    (id : Arch.sys_reg_id) (register : RegisterRef α) : SailM α :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_sys_reg_read id register)

abbrev sail_sys_reg_write [Arch]
    (id : Arch.sys_reg_id) (register : RegisterRef α) (value : α) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_sys_reg_write id register value)

abbrev sail_mem_address_announce [Arch]
    (announcement : Mem_request n nt Arch.addr_size Arch.addr_space Arch.mem_acc) :
    SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_mem_address_announce announcement)

abbrev sail_barrier [Arch] (value : Arch.barrier) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_barrier value)

abbrev sail_cache_op [Arch] (operation : Arch.cache_op) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_cache_op operation)

abbrev sail_tlbi [Arch] (operation : Arch.tlbi) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_tlbi operation)

abbrev sail_translation_start [Arch] (value : Arch.trans_start) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_translation_start value)

abbrev sail_translation_end [Arch] (value : Arch.trans_end) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_translation_end value)

abbrev sail_take_exception [Arch] (exception : Arch.exn) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_take_exception exception)

abbrev sail_return_exception (value : Unit) : SailM Unit :=
  liftBase (PreSail.ConcurrencyInterfaceV2.sail_return_exception value)

end ConcurrencyInterfaceV2

abbrev cycle_count (value : Unit) : SailM Unit :=
  liftBase (PreSail.cycle_count value)

abbrev get_cycle_count (value : Unit) : SailM Nat :=
  liftBase (PreSail.get_cycle_count value)

abbrev print_effect (message : String) : SailM Unit :=
  liftBase (PreSail.print_effect message)

abbrev print_int_effect (message : String) (value : Int) : SailM Unit :=
  liftBase (PreSail.print_int_effect message value)

abbrev print_bits_effect {width : Nat}
    (message : String) (value : BitVec width) : SailM Unit :=
  liftBase (PreSail.print_bits_effect message value)

abbrev print_endline_effect (message : String) : SailM Unit :=
  liftBase (PreSail.print_endline_effect message)

abbrev SailME (error result : Type) :=
  ExceptT (Error exception ⊕ error) SailM result

instance : MonadExceptOf (Error exception) (SailME α) where
  throw error := MonadExcept.throw (.inl error)
  tryCatch action handler :=
    MonadExcept.tryCatch action fun
      | .inl error => handler error
      | .inr value => MonadExcept.throw (.inr value)

namespace SailME

def run (action : SailME α α) : SailM α := do
  match ← ExceptT.run action with
  | .error (.inr value) => pure value
  | .error (.inl error) => throw error
  | .ok value => pure value

def throw (error : α) : SailME α β :=
  MonadExceptOf.throw (Sum.inr (α := Error exception) error)

end SailME

def sailTryCatchE
    (action : SailME β α) (handler : exception → SailME β α) : SailME β α :=
  ExceptT.mk <| sailTryCatch (ExceptT.run action) fun error =>
    ExceptT.run (handler error)

def unwrapValue [Inhabited α] (action : SailM α) : α :=
  match (action.run initialHostState).run default with
  | .ok (value, _) _ => value
  | _ => default

private def hexValue (character : Char) : Nat :=
  if '0' ≤ character ∧ character ≤ '9' then
    character.toNat - '0'.toNat
  else if 'a' ≤ character ∧ character ≤ 'f' then
    character.toNat - 'a'.toNat + 10
  else if 'A' ≤ character ∧ character ≤ 'F' then
    character.toNat - 'A'.toNat + 10
  else
    0

private def parseHex : List Char → Array byte
  | high :: low :: rest =>
      #[BitVec.ofNat 8 (16 * hexValue high + hexValue low)] ++ parseHex rest
  | _ => #[]

private def hexDigit (value : Nat) : Char :=
  match value with
  | 0 => '0' | 1 => '1' | 2 => '2' | 3 => '3'
  | 4 => '4' | 5 => '5' | 6 => '6' | 7 => '7'
  | 8 => '8' | 9 => '9' | 10 => 'a' | 11 => 'b'
  | 12 => 'c' | 13 => 'd' | 14 => 'e' | _ => 'f'

private def renderHex (bytes : Array byte) : String :=
  String.ofList <| bytes.toList.flatMap fun byte =>
    [hexDigit (byte.toNat / 16), hexDigit (byte.toNat % 16)]

def main_of_sail_main
    (initialState :
      PreSail.SequentialState RegisterType trivialChoiceSource)
    (main : Unit → SailM Unit) : IO UInt32 := do
  let inputText ← (← IO.getStdin).readToEnd
  let hostState :=
    { initialHostState with
      inputBytes := parseHex inputText.trimAscii.toString.toList }
  match ((main ()).run hostState).run initialState with
  | .ok (_, finalHostState) finalState => do
      for message in finalState.sailOutput do
        IO.print message
      IO.println (renderHex finalHostState.publicOutput)
      return 0
  | .error error finalState => do
      for message in finalState.sailOutput do
        IO.print message
      IO.eprintln s!"Error while running the Sail program!: {error.print}"
      return 1

def prerr_endline : String → Unit := fun _ => ()
def print : String → Unit := fun _ => ()
def prerr : String → Unit := fun _ => ()

end Evm
