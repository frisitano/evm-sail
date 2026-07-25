import Evm.Primitives.Fork

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

/-! # Chain configuration

The decoded `SszChainConfig`. Its activation point is validated while decoding,
so the resulting value only carries the chain id. The schema evm_prefix selects the
protocol profile, so neither the fork identifier nor its blob schedule is
repeated inside the SSZ body. Pure data — no registers, no externs. -/

def undefined_BlobSchedule (_ : Unit) : SailM BlobSchedule := do
  (pure { target := ← (undefined_range 0 14),
          max := ← (undefined_range 0 21),
          base_fee_update_fraction := ← (undefined_range 1 11684671) })

def undefined_ProtocolProfile (_ : Unit) : SailM ProtocolProfile := do
  (pure { fork := ← (undefined_Fork ()),
          blob_schedule := ← (undefined_BlobSchedule ()) })

/-- The inactive blob schedule used before EIP-4844. -/
def NO_BLOB_SCHEDULE : BlobSchedule :=
  { target := 0,
    max := 0,
    base_fee_update_fraction := 1 }

/-- Resolves the schema's stable `ProtocolFork` byte to the execution-rule
family and its protocol-defined blob schedule. BPO forks share Osaka
execution rules but retain their distinct blob parameters. An unknown
input byte is rejected before any profile field reaches a kernel
register. -/
def protocol_profile (schema_fork : (BitVec 8)) : SailM ProtocolProfile := do
  if ((schema_fork == 0x0A#8) : Bool)
  then
    (pure { fork := Berlin,
            blob_schedule := NO_BLOB_SCHEDULE })
  else
    (do
      if (((schema_fork == 0x0B#8) || ((schema_fork == 0x0C#8) || (schema_fork == 0x0D#8))) : Bool)
      then
        (pure { fork := London,
                blob_schedule := NO_BLOB_SCHEDULE })
      else
        (do
          if ((schema_fork == 0x0E#8) : Bool)
          then
            (pure { fork := Paris,
                    blob_schedule := NO_BLOB_SCHEDULE })
          else
            (do
              if ((schema_fork == 0x0F#8) : Bool)
              then
                (pure { fork := Shanghai,
                        blob_schedule := NO_BLOB_SCHEDULE })
              else
                (do
                  if ((schema_fork == 0x10#8) : Bool)
                  then
                    (pure { fork := Cancun,
                            blob_schedule := { target := 3,
                                               max := 6,
                                               base_fee_update_fraction := 3338477 } })
                  else
                    (do
                      if (((schema_fork == 0x11#8) || (schema_fork == 0x12#8)) : Bool)
                      then
                        (pure { fork := if ((schema_fork == 0x11#8) : Bool)
                                  then Prague
                                  else Osaka,
                                blob_schedule := { target := 6,
                                                   max := 9,
                                                   base_fee_update_fraction := 5007716 } })
                      else
                        (do
                          if ((schema_fork == 0x13#8) : Bool)
                          then
                            (pure { fork := Osaka,
                                    blob_schedule := { target := 10,
                                                       max := 15,
                                                       base_fee_update_fraction := 8346193 } })
                          else
                            (do
                              if ((schema_fork == 0x14#8) : Bool)
                              then
                                (pure { fork := Osaka,
                                        blob_schedule := { target := 14,
                                                           max := 21,
                                                           base_fee_update_fraction := 11684671 } })
                              else
                                (do
                                  if ((schema_fork == 0x15#8) : Bool)
                                  then
                                    (pure { fork := Amsterdam,
                                            blob_schedule := { target := 14,
                                                               max := 21,
                                                               base_fee_update_fraction := 11684671 } })
                                  else sailThrow ((InvalidBlock InvalidConfig))))))))))

def undefined_ChainConfig (_ : Unit) : SailM ChainConfig := do
  (pure { chain_id := ← (undefined_range 0 ((2 ^i 64) - 1)) })

