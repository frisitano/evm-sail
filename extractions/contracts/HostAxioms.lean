import Evm.Defs

open Sail
open ConcurrencyInterfaceV1
open Evm.Defs

namespace Evm.Functions

@[extern "lean_evmsail_ancestor_hash_write"]
axiom ancestor_hash_write : ancestor_index → hash → SailM Unit
@[extern "lean_evmsail_ancestor_hash_read"]
axiom ancestor_hash_read : ancestor_index → SailM hash

@[extern "lean_evmsail_keccak256_segments"]
axiom keccak256_segments : List Bytes → SailM (BitVec 256)
@[extern "lean_evmsail_sha256_segments"]
axiom sha256_segments : List Bytes → SailM (BitVec 256)

@[extern "lean_evmsail_mem_read_byte"]
axiom mem_read_byte : byte_quantity → SailM (BitVec 8)
@[extern "lean_evmsail_mem_write_byte"]
axiom mem_write_byte : byte_quantity → BitVec 8 → SailM Unit
@[extern "lean_evmsail_mem_clear"]
axiom mem_clear : Unit → SailM Unit
@[extern "lean_evmsail_mem_frame_enter"]
axiom mem_frame_enter : Unit → SailM byte_quantity
@[extern "lean_evmsail_mem_frame_leave"]
axiom mem_frame_leave : Unit → SailM Unit
@[extern "lean_evmsail_mem_expand"]
axiom mem_expand : byte_quantity → SailM Bool
@[extern "lean_evmsail_mem_move"]
axiom mem_move : byte_quantity → byte_quantity → byte_quantity → SailM Unit
@[extern "lean_evmsail_mem_load_word"]
axiom mem_load_word : byte_quantity → SailM word
@[extern "lean_evmsail_mem_store_word"]
axiom mem_store_word : byte_quantity → word → SailM Unit

@[extern "lean_evmsail_host_scratch_store_bytes"]
axiom host_scratch_store_bytes : byte_quantity → List (BitVec 8) → byte_quantity → SailM Bool
@[extern "lean_evmsail_host_scratch_store_slice"]
axiom host_scratch_store_slice : byte_quantity → ByteSlice → SailM Bool
@[extern "lean_evmsail_host_scratch_truncate"]
axiom host_scratch_truncate : byte_quantity → SailM Unit

@[extern "lean_evmsail_accelerator_ripemd160"]
axiom accelerator_ripemd160 : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_modexp"]
axiom accelerator_modexp : ByteSlice → byte_length → byte_length → byte_length → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_add"]
axiom accelerator_bn254_add : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_mul"]
axiom accelerator_bn254_mul : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_pairing"]
axiom accelerator_bn254_pairing : ByteSlice → SailM (BitVec 2)
@[extern "lean_evmsail_accelerator_blake2f"]
axiom accelerator_blake2f : ByteSlice → blake2_rounds → y_parity → SailM Bool
@[extern "lean_evmsail_accelerator_kzg_point_evaluation"]
axiom accelerator_kzg_point_evaluation : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g1_add"]
axiom accelerator_bls_g1_add : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g1_msm"]
axiom accelerator_bls_g1_msm : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g2_add"]
axiom accelerator_bls_g2_add : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g2_msm"]
axiom accelerator_bls_g2_msm : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_pairing"]
axiom accelerator_bls_pairing : ByteSlice → SailM (BitVec 2)
@[extern "lean_evmsail_accelerator_bls_map_fp_to_g1"]
axiom accelerator_bls_map_fp_to_g1 : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_map_fp2_to_g2"]
axiom accelerator_bls_map_fp2_to_g2 : ByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_p256_verify"]
axiom accelerator_p256_verify : ByteSlice → SailM Bool
@[extern "lean_evmsail_secp256k1_verify"]
axiom secp256k1_verify : BitVec 256 → word → word → BitVec 256 → BitVec 256 → SailM Bool
@[extern "lean_evmsail_host_ecrecover"]
axiom host_ecrecover : BitVec 256 → y_parity → word → word → SailM (BitVec 168)

@[extern "lean_evmsail_stack_reset"]
axiom stack_reset : Unit → SailM Unit
@[extern "lean_evmsail_stack_enter_frame"]
axiom stack_enter_frame : Unit → SailM Unit
@[extern "lean_evmsail_stack_leave_frame"]
axiom stack_leave_frame : Unit → SailM Unit
@[extern "lean_evmsail_stack_depth"]
axiom stack_depth : Unit → SailM operand_stack_height
@[extern "lean_evmsail_stack_push_word"]
axiom stack_push_word : word → SailM Unit
@[extern "lean_evmsail_stack_pop_word"]
axiom stack_pop_word : Unit → SailM word
@[extern "lean_evmsail_stack_peek_word"]
axiom stack_peek_word : stack_index → SailM word
@[extern "lean_evmsail_stack_set_word"]
axiom stack_set_word : stack_index → word → SailM Unit

@[extern "lean_evmsail_stateless_input"]
axiom stateless_input : Unit → SailM ByteSlice
@[extern "lean_evmsail_host_slice_byte"]
axiom host_slice_byte : ByteSlice → byte_quantity → SailM (BitVec 8)
@[extern "lean_evmsail_host_slice_count_nonzero"]
axiom host_slice_count_nonzero : ByteSlice → SailM byte_quantity
@[extern "lean_evmsail_host_slice_strided_zero"]
axiom host_slice_strided_zero :
  ByteSlice → byte_quantity → byte_quantity → byte_quantity → byte_quantity → SailM Bool
@[extern "lean_evmsail_host_slice_load_word"]
axiom host_slice_load_word : ByteSlice → byte_quantity → SailM word
@[extern "lean_evmsail_host_slice_load_n_word"]
axiom host_slice_load_n_word : ByteSlice → byte_quantity → byte_quantity → SailM word
@[extern "lean_evmsail_host_slice_copy_to_memory"]
axiom host_slice_copy_to_memory :
  ByteSlice → byte_quantity → byte_quantity → byte_quantity → SailM Unit
@[extern "lean_evmsail_bytes_segments_equal_slice"]
axiom bytes_segments_equal_slice : List Bytes → ByteSlice → SailM Bool

@[extern "lean_evmsail_output_buffer_store"]
axiom output_buffer_store : ByteSlice → SailM Bool
@[extern "lean_evmsail_output_buffer_store_word"]
axiom output_buffer_store_word : word → SailM Bool
@[extern "lean_evmsail_output_buffer_store_words"]
axiom output_buffer_store_words : word → word → SailM Bool
@[extern "lean_evmsail_public_output_write"]
axiom public_output_write : ByteSlice → SailM Bool

@[extern "lean_evmsail_code_db_lookup"]
axiom code_db_lookup : hash → SailM (Option Code)
@[extern "lean_evmsail_jumpdest_table_alloc"]
axiom jumpdest_table_alloc : byte_quantity → SailM JumpdestRef
@[extern "lean_evmsail_jumpdest_table_store_chunk"]
axiom jumpdest_table_store_chunk :
  JumpdestRef → byte_quantity → byte_quantity → JumpdestChunk → SailM Bool
@[extern "lean_evmsail_code_db_store"]
axiom code_db_store : ByteSlice → JumpdestRef → SailM hash
@[extern "lean_evmsail_jumpdest_ref_contains"]
axiom jumpdest_ref_contains : JumpdestRef → byte_quantity → byte_quantity → SailM Bool
@[extern "lean_evmsail_code_intern_delegation"]
axiom code_intern_delegation : address → JumpdestRef → SailM hash
@[extern "lean_evmsail_code_db_read_delegation"]
axiom code_db_read_delegation : hash → SailM (BitVec 168)

@[extern "lean_evmsail_transient_reset"]
axiom transient_reset : Unit → SailM Unit
@[extern "lean_evmsail_transient_store"]
axiom transient_store : address → word → word → SailM Unit
@[extern "lean_evmsail_transient_load"]
axiom transient_load : address → word → SailM word

@[extern "lean_evmsail_storage_tx_update"]
axiom storage_tx_update : StorageEntry → SailM Unit
@[extern "lean_evmsail_storage_tx_get"]
axiom storage_tx_get : StorageKey → SailM (Option StorageValue)
@[extern "lean_evmsail_storage_tx_pop"]
axiom storage_tx_pop : Unit → SailM (Option StorageEntry)
@[extern "lean_evmsail_storage_tx_checkpoint"]
axiom storage_tx_checkpoint : Unit → SailM StorageCheckpoint
@[extern "lean_evmsail_storage_tx_revert"]
axiom storage_tx_revert : StorageCheckpoint → SailM Unit
@[extern "lean_evmsail_storage_tx_clear"]
axiom storage_tx_clear : address → SailM Unit
@[extern "lean_evmsail_storage_tx_reset"]
axiom storage_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_storage_has_writes"]
axiom storage_has_writes : address → SailM Bool
@[extern "lean_evmsail_storage_block_get"]
axiom storage_block_get : StorageKey → SailM (Option StorageValue)
@[extern "lean_evmsail_storage_block_put"]
axiom storage_block_put : StorageEntry → SailM Unit
@[extern "lean_evmsail_storage_block_cache"]
axiom storage_block_cache : StorageKey → word → SailM Unit
@[extern "lean_evmsail_storage_block_clear"]
axiom storage_block_clear : address → SailM Unit
@[extern "lean_evmsail_storage_block_count"]
axiom storage_block_count : address → SailM item_count
@[extern "lean_evmsail_storage_block_row"]
axiom storage_block_row : address → item_index → SailM (Option StorageEntry)

@[extern "lean_evmsail_acct_tx_get"]
axiom acct_tx_get : address → SailM (Option Account)
@[extern "lean_evmsail_acct_tx_update"]
axiom acct_tx_update : address → Account → SailM Unit
@[extern "lean_evmsail_acct_tx_set_balance"]
axiom acct_tx_set_balance : address → word → SailM Unit
@[extern "lean_evmsail_acct_tx_set_nonce"]
axiom acct_tx_set_nonce : address → account_nonce → SailM Unit
@[extern "lean_evmsail_acct_tx_set_code_hash"]
axiom acct_tx_set_code_hash : address → hash → SailM Unit
@[extern "lean_evmsail_acct_tx_pop_ascending"]
axiom acct_tx_pop_ascending : Unit → SailM (Option AcctEntry)
@[extern "lean_evmsail_acct_tx_checkpoint"]
axiom acct_tx_checkpoint : Unit → SailM AccountCheckpoint
@[extern "lean_evmsail_acct_tx_revert"]
axiom acct_tx_revert : AccountCheckpoint → SailM Unit
@[extern "lean_evmsail_acct_tx_reset"]
axiom acct_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_acct_block_get"]
axiom acct_block_get : address → SailM (Option Account)
@[extern "lean_evmsail_acct_block_write"]
axiom acct_block_write : AcctEntry → SailM Unit
@[extern "lean_evmsail_acct_block_cache"]
axiom acct_block_cache : address → Account → SailM Unit
@[extern "lean_evmsail_acct_block_count"]
axiom acct_block_count : Unit → SailM item_count
@[extern "lean_evmsail_acct_block_row"]
axiom acct_block_row : item_index → SailM (Option AcctEntry)

@[extern "lean_evmsail_bal_reset"]
axiom bal_reset : Unit → SailM Unit
@[extern "lean_evmsail_bal_set_index"]
axiom bal_set_index : item_index → SailM Unit
@[extern "lean_evmsail_bal_account_touch"]
axiom bal_account_touch : address → SailM Unit
@[extern "lean_evmsail_bal_storage_change"]
axiom bal_storage_change : address → word → word → SailM Unit
@[extern "lean_evmsail_bal_storage_read"]
axiom bal_storage_read : address → word → SailM Unit
@[extern "lean_evmsail_bal_balance_change"]
axiom bal_balance_change : address → word → SailM Unit
@[extern "lean_evmsail_bal_nonce_change"]
axiom bal_nonce_change : address → account_nonce → SailM Unit
@[extern "lean_evmsail_bal_code_change"]
axiom bal_code_change : address → hash → SailM Unit
@[extern "lean_evmsail_bal_prepare"]
axiom bal_prepare : Unit → SailM Unit
@[extern "lean_evmsail_bal_account_count"]
axiom bal_account_count : Unit → SailM item_count
@[extern "lean_evmsail_bal_account_address"]
axiom bal_account_address : item_index → SailM address
@[extern "lean_evmsail_bal_storage_change_count"]
axiom bal_storage_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_storage_change_slot"]
axiom bal_storage_change_slot : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_storage_change_index"]
axiom bal_storage_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_storage_change_value"]
axiom bal_storage_change_value : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_storage_read_count"]
axiom bal_storage_read_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_storage_read_slot"]
axiom bal_storage_read_slot : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_balance_change_count"]
axiom bal_balance_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_balance_change_index"]
axiom bal_balance_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_balance_change_value"]
axiom bal_balance_change_value : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_nonce_change_count"]
axiom bal_nonce_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_nonce_change_index"]
axiom bal_nonce_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_nonce_change_value"]
axiom bal_nonce_change_value : item_index → item_index → SailM account_nonce
@[extern "lean_evmsail_bal_code_change_count"]
axiom bal_code_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_code_change_index"]
axiom bal_code_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_code_change_hash"]
axiom bal_code_change_hash : item_index → item_index → SailM hash

@[extern "lean_evmsail_warm_reset"]
axiom warm_reset : Unit → SailM Unit
@[extern "lean_evmsail_warm_addr_touch"]
axiom warm_addr_touch : address → SailM Bool
@[extern "lean_evmsail_warm_addr_remove"]
axiom warm_addr_remove : address → SailM Unit
@[extern "lean_evmsail_warm_slot_touch"]
axiom warm_slot_touch : address → word → SailM Bool
@[extern "lean_evmsail_warm_slot_remove"]
axiom warm_slot_remove : address → word → SailM Unit

@[extern "lean_evmsail_logs_tx_reset"]
axiom logs_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_log_append"]
axiom log_append : address → List word → Bytes → SailM Unit
@[extern "lean_evmsail_logs_checkpoint"]
axiom logs_checkpoint : Unit → SailM LogCheckpoint
@[extern "lean_evmsail_logs_revert"]
axiom logs_revert : LogCheckpoint → SailM Unit
@[extern "lean_evmsail_read_logs"]
axiom read_logs : Unit → SailM (List LogEntry)

@[extern "lean_evmsail_journal_reset"]
axiom journal_reset : Unit → SailM Unit
@[extern "lean_evmsail_journal_len"]
axiom journal_len : Unit → SailM JournalCheckpoint
@[extern "lean_evmsail_journal_push"]
axiom journal_push : JEntry → SailM Unit
@[extern "lean_evmsail_journal_pop"]
axiom journal_pop : Unit → SailM JEntry

@[extern "lean_evmsail_nodedb_reset"]
axiom nodedb_reset : Unit → SailM Unit
@[extern "lean_evmsail_nodedb_insert"]
axiom nodedb_insert : hash → source_pointer → byte_length → SailM Unit
@[extern "lean_evmsail_nodedb_off"]
axiom nodedb_off : hash → SailM source_pointer
@[extern "lean_evmsail_nodedb_len"]
axiom nodedb_len : hash → SailM byte_length

end Evm.Functions
