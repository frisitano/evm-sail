import Evm.Defs

open Sail
open ConcurrencyInterfaceV1
open Evm.Defs

namespace Evm.Functions

axiom ancestor_hash_write : ancestor_index → hash → SailM Unit
axiom ancestor_hash_read : ancestor_index → SailM hash

axiom keccak256_segments : List Bytes → SailM (BitVec 256)
axiom sha256_segments : List Bytes → SailM (BitVec 256)

axiom mem_read_byte : byte_quantity → SailM (BitVec 8)
axiom mem_write_byte : byte_quantity → BitVec 8 → SailM Unit
axiom mem_clear : Unit → SailM Unit
axiom mem_frame_enter : Unit → SailM byte_quantity
axiom mem_frame_leave : Unit → SailM Unit
axiom mem_expand : byte_quantity → SailM Bool
axiom mem_move : byte_quantity → byte_quantity → byte_quantity → SailM Unit
axiom mem_load_word : byte_quantity → SailM word
axiom mem_store_word : byte_quantity → word → SailM Unit

axiom host_scratch_store_bytes : byte_quantity → List (BitVec 8) → byte_quantity → SailM Bool
axiom host_scratch_store_slice : byte_quantity → EvmByteSlice → SailM Bool
axiom host_scratch_truncate : byte_quantity → SailM Unit

axiom accelerator_ripemd160 : EvmByteSlice → SailM Bool
axiom accelerator_modexp : EvmByteSlice → byte_length → byte_length → byte_length → SailM Bool
axiom accelerator_bn254_add : EvmByteSlice → SailM Bool
axiom accelerator_bn254_mul : EvmByteSlice → SailM Bool
axiom accelerator_bn254_pairing : EvmByteSlice → SailM (BitVec 2)
axiom accelerator_blake2f : EvmByteSlice → blake2_rounds → y_parity → SailM Bool
axiom accelerator_kzg_point_evaluation : EvmByteSlice → SailM Bool
axiom accelerator_bls_g1_add : EvmByteSlice → SailM Bool
axiom accelerator_bls_g1_msm : EvmByteSlice → SailM Bool
axiom accelerator_bls_g2_add : EvmByteSlice → SailM Bool
axiom accelerator_bls_g2_msm : EvmByteSlice → SailM Bool
axiom accelerator_bls_pairing : EvmByteSlice → SailM (BitVec 2)
axiom accelerator_bls_map_fp_to_g1 : EvmByteSlice → SailM Bool
axiom accelerator_bls_map_fp2_to_g2 : EvmByteSlice → SailM Bool
axiom accelerator_p256_verify : EvmByteSlice → SailM Bool
axiom secp256k1_verify : BitVec 256 → word → word → BitVec 256 → BitVec 256 → SailM Bool
axiom host_ecrecover : BitVec 256 → y_parity → word → word → SailM (BitVec 168)

axiom stack_reset : Unit → SailM Unit
axiom stack_enter_frame : Unit → SailM Unit
axiom stack_leave_frame : Unit → SailM Unit
axiom stack_depth : Unit → SailM operand_stack_height
axiom stack_push_word : word → SailM Unit
axiom stack_pop_word : Unit → SailM word
axiom stack_peek_word : stack_index → SailM word
axiom stack_set_word : stack_index → word → SailM Unit

axiom stateless_input : Unit → SailM EvmByteSlice
axiom host_slice_byte : EvmByteSlice → byte_quantity → SailM (BitVec 8)
axiom host_slice_count_nonzero : EvmByteSlice → SailM byte_quantity
axiom host_slice_strided_zero :
  EvmByteSlice → byte_quantity → byte_quantity → byte_quantity → byte_quantity → SailM Bool
axiom host_slice_load_word : EvmByteSlice → byte_quantity → SailM word
axiom host_slice_load_n_word : EvmByteSlice → byte_quantity → byte_quantity → SailM word
axiom host_slice_copy_to_memory :
  EvmByteSlice → byte_quantity → byte_quantity → byte_quantity → SailM Unit
axiom bytes_segments_equal_slice : List Bytes → EvmByteSlice → SailM Bool

axiom output_buffer_store : EvmByteSlice → SailM Bool
axiom output_buffer_store_word : word → SailM Bool
axiom output_buffer_store_words : word → word → SailM Bool
axiom public_output_write : EvmByteSlice → SailM Bool

axiom code_db_lookup : hash → SailM (Option Code)
axiom jumpdest_table_alloc : byte_quantity → SailM JumpdestRef
axiom jumpdest_table_store_chunk :
  JumpdestRef → byte_quantity → byte_quantity → JumpdestChunk → SailM Bool
axiom code_db_store : EvmByteSlice → JumpdestRef → SailM hash
axiom jumpdest_ref_contains : JumpdestRef → byte_quantity → byte_quantity → SailM Bool
axiom code_intern_delegation : address → JumpdestRef → SailM hash
axiom code_db_read_delegation : hash → SailM (BitVec 168)

axiom transient_reset : Unit → SailM Unit
axiom transient_store : address → word → word → SailM Unit
axiom transient_load : address → word → SailM word

axiom storage_tx_update : StorageEntry → SailM Unit
axiom storage_tx_get : StorageKey → SailM (Option StorageValue)
axiom storage_tx_pop : Unit → SailM (Option StorageEntry)
axiom storage_tx_checkpoint : Unit → SailM StorageCheckpoint
axiom storage_tx_revert : StorageCheckpoint → SailM Unit
axiom storage_tx_clear : address → SailM Unit
axiom storage_tx_reset : Unit → SailM Unit
axiom storage_has_writes : address → SailM Bool
axiom storage_block_get : StorageKey → SailM (Option StorageValue)
axiom storage_block_put : StorageEntry → SailM Unit
axiom storage_block_cache : StorageKey → word → SailM Unit
axiom storage_block_clear : address → SailM Unit
axiom storage_block_count : address → SailM item_count
axiom storage_block_row : address → item_index → SailM (Option StorageEntry)

axiom acct_tx_get : address → SailM (Option Account)
axiom acct_tx_update : address → Account → SailM Unit
axiom acct_tx_set_balance : address → word → SailM Unit
axiom acct_tx_set_nonce : address → account_nonce → SailM Unit
axiom acct_tx_set_code_hash : address → hash → SailM Unit
axiom acct_tx_pop_ascending : Unit → SailM (Option AcctEntry)
axiom acct_tx_checkpoint : Unit → SailM AccountCheckpoint
axiom acct_tx_revert : AccountCheckpoint → SailM Unit
axiom acct_tx_reset : Unit → SailM Unit
axiom acct_block_get : address → SailM (Option Account)
axiom acct_block_write : AcctEntry → SailM Unit
axiom acct_block_cache : address → Account → SailM Unit
axiom acct_block_count : Unit → SailM item_count
axiom acct_block_row : item_index → SailM (Option AcctEntry)

axiom bal_reset : Unit → SailM Unit
axiom bal_set_index : item_index → SailM Unit
axiom bal_account_touch : address → SailM Unit
axiom bal_storage_change : address → word → word → SailM Unit
axiom bal_storage_read : address → word → SailM Unit
axiom bal_balance_change : address → word → SailM Unit
axiom bal_nonce_change : address → account_nonce → SailM Unit
axiom bal_code_change : address → hash → SailM Unit
axiom bal_prepare : Unit → SailM Unit
axiom bal_account_count : Unit → SailM item_count
axiom bal_account_address : item_index → SailM address
axiom bal_storage_change_count : item_index → SailM item_count
axiom bal_storage_change_slot : item_index → item_index → SailM word
axiom bal_storage_change_index : item_index → item_index → SailM item_index
axiom bal_storage_change_value : item_index → item_index → SailM word
axiom bal_storage_read_count : item_index → SailM item_count
axiom bal_storage_read_slot : item_index → item_index → SailM word
axiom bal_balance_change_count : item_index → SailM item_count
axiom bal_balance_change_index : item_index → item_index → SailM item_index
axiom bal_balance_change_value : item_index → item_index → SailM word
axiom bal_nonce_change_count : item_index → SailM item_count
axiom bal_nonce_change_index : item_index → item_index → SailM item_index
axiom bal_nonce_change_value : item_index → item_index → SailM account_nonce
axiom bal_code_change_count : item_index → SailM item_count
axiom bal_code_change_index : item_index → item_index → SailM item_index
axiom bal_code_change_hash : item_index → item_index → SailM hash

axiom warm_reset : Unit → SailM Unit
axiom warm_addr_touch : address → SailM Bool
axiom warm_addr_remove : address → SailM Unit
axiom warm_slot_touch : address → word → SailM Bool
axiom warm_slot_remove : address → word → SailM Unit

axiom logs_tx_reset : Unit → SailM Unit
axiom log_append : address → List word → Bytes → SailM Unit
axiom logs_checkpoint : Unit → SailM LogCheckpoint
axiom logs_revert : LogCheckpoint → SailM Unit
axiom read_logs : Unit → SailM (List LogEntry)

axiom journal_reset : Unit → SailM Unit
axiom journal_len : Unit → SailM JournalCheckpoint
axiom journal_push : JEntry → SailM Unit
axiom journal_pop : Unit → SailM JEntry

axiom nodedb_reset : Unit → SailM Unit
axiom nodedb_insert : hash → source_pointer → byte_length → SailM Unit
axiom nodedb_off : hash → SailM source_pointer
axiom nodedb_len : hash → SailM byte_length

end Evm.Functions
