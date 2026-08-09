import Evm

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000

open Sail
open Evm
open Evm.Defs
open Evm.Defs.Register
open Evm.Functions

namespace EvmLeanRunner

@[extern "lean_internal_set_thread_stack_size"]
private opaque setThreadStackSize (size : USize) : BaseIO Unit

private def runnerStackBytes : USize := 60 * 1024 * 1024

private def initialSequentialState :
    PreSail.SequentialState RegisterType trivialChoiceSource :=
  ⟨default, (), default, default, default, default⟩

private def modelBytes (input : ByteArray) : Array byte :=
  input.data.map fun value => BitVec.ofNat 8 value.toNat

private def nativeBytes (input : Array byte) : ByteArray :=
  ⟨input.map fun value => UInt8.ofNat value.toNat⟩

private structure LastRun where
  input : ByteArray := {}
  output : ByteArray := {}
  completed : Bool := false
  deriving Inhabited

initialize lastRun : IO.Ref LastRun ← IO.mkRef {}

private def execute (input : ByteArray) : ByteArray × Bool :=
  let hostState :=
    { initialHostState with
      inputBytes := modelBytes input }
  let action : Evm.SailM Unit := do
    sail_model_init ()
    sail_main ()
  match (action.run hostState).run initialSequentialState with
  | .ok (_, finalHostState) _ =>
      (nativeBytes finalHostState.publicOutput, true)
  | .error _ _ =>
      ({}, false)

@[export evmsail_lean_run_once]
def runOnce (input : ByteArray) : IO ByteArray := do
  setThreadStackSize runnerStackBytes
  let (output, completed) := execute input
  lastRun.set { input, output, completed }
  pure output

private structure ValidationDiagnostic where
  failed : Bool := false
  scope : UInt8 := 0
  reason : UInt8 := 0

/-- The extracted `fatal_error` throws `Error.Exit` and discards its reason,
so a failed run can be reported but not attributed.  `num_of_FatalError` is
the model's own encoding, kept here so the mapping stays in one place if the
reason ever reaches the boundary. -/
private def reasonIndex (reason : FatalError) : UInt8 :=
  UInt8.ofNat (num_of_FatalError reason)

/-- Re-runs the model to classify a failure.  This mirrors `sail_main`
exactly rather than restating its steps: the guest entry is a single
`verify_stateless_payload` call, so there are no intermediate stages left to
bisect and `scope` is no longer meaningful. -/
private def diagnose (input : ByteArray) : ValidationDiagnostic :=
  let hostState :=
    { initialHostState with
      inputBytes := modelBytes input }
  let action : Evm.SailM Unit := do
    sail_model_init ()
    sail_main ()
  match (action.run hostState).run initialSequentialState with
  | .ok _ _ => {}
  | .error _ _ =>
      { failed := true
        scope := 0
        reason := reasonIndex .ExecutionInvalid }

private def pushZeros (output : ByteArray) (count : Nat) : ByteArray :=
  (List.range count).foldl (fun result _ => result.push 0) output

private def pushU32BE (output : ByteArray) (value : Nat) : ByteArray :=
  output
    |>.push (UInt8.ofNat (value / 0x1000000))
    |>.push (UInt8.ofNat (value / 0x10000))
    |>.push (UInt8.ofNat (value / 0x100))
    |>.push (UInt8.ofNat value)

private def appendBytes (output bytes : ByteArray) : ByteArray :=
  bytes.data.foldl ByteArray.push output

private def snapshot
    (run : LastRun) (diagnostic : ValidationDiagnostic) : ByteArray :=
  let output := ByteArray.empty
    |>.push (UInt8.ofNat 'G'.toNat)
    |>.push (if run.completed then 1 else 0)
  let output := pushZeros output 32
  let output :=
    if run.completed then output
    else output.push 0xff |>.push 0 |>.push 0
  let output := pushZeros output 32
  let output := output.push (UInt8.ofNat 'O'.toNat)
  let output := pushU32BE output run.output.size
  let output := appendBytes output run.output
  let output := output.push (UInt8.ofNat 'V'.toNat)
  let output :=
    if diagnostic.failed then
      output.push 1 |>.push diagnostic.scope |>.push diagnostic.reason
    else
      output.push 0
  let output := output.push (UInt8.ofNat 'A'.toNat)
  let output := pushU32BE output 0
  let output := output.push (UInt8.ofNat 'S'.toNat)
  let output := pushU32BE output 0
  let output := output.push (UInt8.ofNat 'M'.toNat)
  let output := pushU32BE output 0
  output.push (UInt8.ofNat 'E'.toNat)

@[export evmsail_lean_debug_dump]
def debugDump (_ : Unit) : IO ByteArray := do
  let run ← lastRun.get
  pure (snapshot run (diagnose run.input))

end EvmLeanRunner
