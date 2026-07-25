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

private def blockErrorIndex : BlockError → UInt8
  | .InvalidConfig => 0
  | .HeaderChainBroken => 1
  | .RlpDecode => 2
  | .InvalidSignature => 3
  | .InvalidGasLimit => 4
  | .GasUsedExceedsLimit => 5
  | .BlobGasLimitExceeded => 6
  | .ExecutionInvalid => 7
  | .InvalidGasUsed => 8
  | .InvalidBlobGasUsed => 9
  | .InvalidExcessBlobGas => 10
  | .InvalidStateRoot => 11
  | .InvalidReceiptsRoot => 12
  | .InvalidLogsBloom => 13
  | .InvalidBlockHash => 14
  | .InvalidParentHash => 15
  | .BlockAccessListTooLarge => 16
  | .InvalidBlockAccessList => 17
  | .InvalidExecutionRequests => 18
  | .WitnessDeficient => 19

private structure ValidationDiagnostic where
  failed : Bool := false
  scope : UInt8 := 0
  reason : UInt8 := 0

private def stageCompletes (inputBytes : ByteArray) (stage : Nat) : Bool :=
  let hostState :=
    { initialHostState with
      inputBytes := modelBytes inputBytes }
  let action : Evm.SailM Unit := do
    sail_model_init ()
    let inputRef ← decode_stateless_input_ref (← stateless_input ())
    if stage == 0 then
      pure ()
    else
      let input ← decode_stateless_input inputRef
      writeReg k_fork inputRef.protocol.fork
      if stage == 1 then
        pure ()
      else
        let witness ← index_execution_witness inputRef
        if stage == 2 then
          pure ()
        else
          validate_execution_payload input inputRef witness
          if stage == 3 then
            pure ()
          else
            let block := input.payload.block'
            bal_reset ()
            writeReg k_block_access_index 0
            if fork_gteq (← readReg k_fork) .Cancun then
              system_call BEACON_ROOTS_ADDR
                (← readReg k_header).parent_beacon_block_root
            else
              pure ()
            if stage == 4 then
              pure ()
            else
              if fork_gteq (← readReg k_fork) .Prague then
                system_call HISTORY_STORAGE_ADDR (← readReg k_header).parent_hash
              else
                pure ()
              if stage == 5 then
                pure ()
              else
                let result ←
                  execute_block_transactions block.body.transactions
                    inputRef.public_keys block.header.gas_limit
                if stage == 6 then
                  pure ()
                else
                  writeReg k_block_access_index
                    (block.body.transactions.count + 1)
                  apply_block_end_state block.body
                  if stage == 7 then
                    pure ()
                  else
                    let requests ←
                      if fork_gteq (← readReg k_fork) .Prague then
                        collect_execution_requests result.deposits
                      else
                        pure EMPTY_EXECUTION_REQUESTS
                    if stage == 8 then
                      pure ()
                    else
                      acct_block_iter_begin ()
                      if stage == 9 then
                        pure ()
                      else
                        prepare_changed_account_post_storage_roots ()
                        if stage == 10 then
                          pure ()
                        else
                          acct_block_iter_begin ()
                          let _ ←
                            trie_root (← readReg k_parent_state_root)
                              (.ChangedAccountTrieUpdates ())
                          if stage == 11 then
                            pure ()
                          else
                            validate_executed_block block inputRef
                              { result with requests := requests }
  match (action.run hostState).run initialSequentialState with
  | .ok _ _ => true
  | .error _ _ => false

private def failingScope (input : ByteArray) : UInt8 :=
  if !stageCompletes input 0 then 1
  else if !stageCompletes input 1 then 1
  else if !stageCompletes input 2 then 2
  else if !stageCompletes input 3 then 3
  else if !stageCompletes input 4 then 4
  else if !stageCompletes input 5 then 4
  else if !stageCompletes input 6 then 4
  else if !stageCompletes input 7 then 4
  else if !stageCompletes input 8 then 4
  else if !stageCompletes input 9 then 5
  else if !stageCompletes input 10 then 5
  else if !stageCompletes input 11 then 5
  else if !stageCompletes input 12 then 5
  else 0

private def diagnose (input : ByteArray) : ValidationDiagnostic :=
  let hostState :=
    { initialHostState with
      inputBytes := modelBytes input }
  let action : Evm.SailM StatelessValidationResult := do
    sail_model_init ()
    let inputRef ← decode_stateless_input_ref (← stateless_input ())
    verify_stateless_payload inputRef
  match (action.run hostState).run initialSequentialState with
  | .ok (.StatelessPayloadValid (), _) _ =>
      {}
  | .ok (.StatelessPayloadInvalid failure, _) _ =>
      { failed := true
        scope := failingScope input
        reason := blockErrorIndex failure.reason }
  | .error (.User (.InvalidBlock reason)) _ =>
      { failed := true
        scope := failingScope input
        reason := blockErrorIndex reason }
  | .error _ _ =>
      { failed := true
        scope := 0
        reason := blockErrorIndex .ExecutionInvalid }

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
