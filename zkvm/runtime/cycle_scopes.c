#include "cycle_scopes.h"

#if defined(__riscv) && defined(EVMSAIL_PLATFORM_ZISK)
typedef struct {
    const char *data;
    uint64_t length;
} zisk_profile_tag;

#define ZISK_PROFILE_TAG(name) \
    static const char name##_bytes[] = #name; \
    static const zisk_profile_tag name##_tag = {name##_bytes, sizeof(name##_bytes) - 1}

ZISK_PROFILE_TAG(stateless_validation);
ZISK_PROFILE_TAG(decode_input);
ZISK_PROFILE_TAG(index_witness);
ZISK_PROFILE_TAG(validate_payload);
ZISK_PROFILE_TAG(execute_block);
ZISK_PROFILE_TAG(validate_result);
ZISK_PROFILE_TAG(compute_output_root);
ZISK_PROFILE_TAG(serialize_output);
ZISK_PROFILE_TAG(block_start);
ZISK_PROFILE_TAG(block_transactions);
ZISK_PROFILE_TAG(tx_decode);
ZISK_PROFILE_TAG(tx_reset);
ZISK_PROFILE_TAG(tx_validate);
ZISK_PROFILE_TAG(tx_upfront);
ZISK_PROFILE_TAG(tx_frame);
ZISK_PROFILE_TAG(tx_settle);
ZISK_PROFILE_TAG(receipts_root);
ZISK_PROFILE_TAG(block_end_state);
ZISK_PROFILE_TAG(block_end_requests);
ZISK_PROFILE_TAG(state_root);
ZISK_PROFILE_TAG(block_access_list);
ZISK_PROFILE_TAG(htr_execution_payload);
ZISK_PROFILE_TAG(htr_transactions);
ZISK_PROFILE_TAG(htr_withdrawals);
ZISK_PROFILE_TAG(htr_versioned_hashes);
ZISK_PROFILE_TAG(htr_execution_requests);
ZISK_PROFILE_TAG(htr_bytes_root);
ZISK_PROFILE_TAG(htr_merkle_padding);
ZISK_PROFILE_TAG(account_load);
ZISK_PROFILE_TAG(account_tx_lookup);
ZISK_PROFILE_TAG(account_block_lookup);
ZISK_PROFILE_TAG(account_witness);
ZISK_PROFILE_TAG(storage_load);
ZISK_PROFILE_TAG(storage_tx_lookup);
ZISK_PROFILE_TAG(storage_block_lookup);
ZISK_PROFILE_TAG(storage_witness);
ZISK_PROFILE_TAG(bal_account_touch);
ZISK_PROFILE_TAG(bal_storage_read);
ZISK_PROFILE_TAG(index_witness_nodes);
ZISK_PROFILE_TAG(index_witness_codes);
ZISK_PROFILE_TAG(index_witness_headers);
ZISK_PROFILE_TAG(request_withdrawals);
ZISK_PROFILE_TAG(request_consolidations);
ZISK_PROFILE_TAG(request_builder_deposits);
ZISK_PROFILE_TAG(request_builder_exits);
ZISK_PROFILE_TAG(account_mutation);
ZISK_PROFILE_TAG(storage_mutation);
ZISK_PROFILE_TAG(tx_merge);
ZISK_PROFILE_TAG(tx_merge_accounts);
ZISK_PROFILE_TAG(tx_merge_storage);
ZISK_PROFILE_TAG(system_call_interpret);
ZISK_PROFILE_TAG(system_call_merge);

#define ZISK_PROFILE_REPORT_STEPS(tag, command) \
    __asm__ volatile( \
        "csrs 0x81a, %0\n\t" \
        "addi x0, x0, " #command \
        : \
        : "r" (&tag##_tag) \
        : "memory")

#define START_SCOPE(id, tag) case id: ZISK_PROFILE_REPORT_STEPS(tag, 7); break
#define END_SCOPE(id, tag) case id: ZISK_PROFILE_REPORT_STEPS(tag, 8); break

#elif defined(__riscv)
__asm__(
    ".globl __ZISKOS_PROFILE_ID_0_stateless_validation\n"
    ".set __ZISKOS_PROFILE_ID_0_stateless_validation, 0\n"
    ".globl __ZISKOS_PROFILE_ID_1_decode_input\n"
    ".set __ZISKOS_PROFILE_ID_1_decode_input, 1\n"
    ".globl __ZISKOS_PROFILE_ID_2_index_witness\n"
    ".set __ZISKOS_PROFILE_ID_2_index_witness, 2\n"
    ".globl __ZISKOS_PROFILE_ID_3_validate_payload\n"
    ".set __ZISKOS_PROFILE_ID_3_validate_payload, 3\n"
    ".globl __ZISKOS_PROFILE_ID_4_execute_block\n"
    ".set __ZISKOS_PROFILE_ID_4_execute_block, 4\n"
    ".globl __ZISKOS_PROFILE_ID_5_validate_result\n"
    ".set __ZISKOS_PROFILE_ID_5_validate_result, 5\n"
    ".globl __ZISKOS_PROFILE_ID_6_compute_output_root\n"
    ".set __ZISKOS_PROFILE_ID_6_compute_output_root, 6\n"
    ".globl __ZISKOS_PROFILE_ID_7_serialize_output\n"
    ".set __ZISKOS_PROFILE_ID_7_serialize_output, 7\n"
    ".globl __ZISKOS_PROFILE_ID_8_block_start\n"
    ".set __ZISKOS_PROFILE_ID_8_block_start, 8\n"
    ".globl __ZISKOS_PROFILE_ID_9_block_transactions\n"
    ".set __ZISKOS_PROFILE_ID_9_block_transactions, 9\n"
    ".globl __ZISKOS_PROFILE_ID_10_tx_decode\n"
    ".set __ZISKOS_PROFILE_ID_10_tx_decode, 10\n"
    ".globl __ZISKOS_PROFILE_ID_11_tx_reset\n"
    ".set __ZISKOS_PROFILE_ID_11_tx_reset, 11\n"
    ".globl __ZISKOS_PROFILE_ID_12_tx_validate\n"
    ".set __ZISKOS_PROFILE_ID_12_tx_validate, 12\n"
    ".globl __ZISKOS_PROFILE_ID_13_tx_upfront\n"
    ".set __ZISKOS_PROFILE_ID_13_tx_upfront, 13\n"
    ".globl __ZISKOS_PROFILE_ID_14_tx_frame\n"
    ".set __ZISKOS_PROFILE_ID_14_tx_frame, 14\n"
    ".globl __ZISKOS_PROFILE_ID_15_tx_settle\n"
    ".set __ZISKOS_PROFILE_ID_15_tx_settle, 15\n"
    ".globl __ZISKOS_PROFILE_ID_16_receipts_root\n"
    ".set __ZISKOS_PROFILE_ID_16_receipts_root, 16\n"
    ".globl __ZISKOS_PROFILE_ID_17_block_end_state\n"
    ".set __ZISKOS_PROFILE_ID_17_block_end_state, 17\n"
    ".globl __ZISKOS_PROFILE_ID_18_block_end_requests\n"
    ".set __ZISKOS_PROFILE_ID_18_block_end_requests, 18\n"
    ".globl __ZISKOS_PROFILE_ID_19_state_root\n"
    ".set __ZISKOS_PROFILE_ID_19_state_root, 19\n"
    ".globl __ZISKOS_PROFILE_ID_20_block_access_list\n"
    ".set __ZISKOS_PROFILE_ID_20_block_access_list, 20\n"
    ".globl __ZISKOS_PROFILE_ID_21_htr_execution_payload\n"
    ".set __ZISKOS_PROFILE_ID_21_htr_execution_payload, 21\n"
    ".globl __ZISKOS_PROFILE_ID_22_htr_transactions\n"
    ".set __ZISKOS_PROFILE_ID_22_htr_transactions, 22\n"
    ".globl __ZISKOS_PROFILE_ID_23_htr_withdrawals\n"
    ".set __ZISKOS_PROFILE_ID_23_htr_withdrawals, 23\n"
    ".globl __ZISKOS_PROFILE_ID_24_htr_versioned_hashes\n"
    ".set __ZISKOS_PROFILE_ID_24_htr_versioned_hashes, 24\n"
    ".globl __ZISKOS_PROFILE_ID_25_htr_execution_requests\n"
    ".set __ZISKOS_PROFILE_ID_25_htr_execution_requests, 25\n"
    ".globl __ZISKOS_PROFILE_ID_26_htr_bytes_root\n"
    ".set __ZISKOS_PROFILE_ID_26_htr_bytes_root, 26\n"
    ".globl __ZISKOS_PROFILE_ID_27_htr_merkle_padding\n"
    ".set __ZISKOS_PROFILE_ID_27_htr_merkle_padding, 27\n"
    ".globl __ZISKOS_PROFILE_ID_28_account_load\n"
    ".set __ZISKOS_PROFILE_ID_28_account_load, 28\n"
    ".globl __ZISKOS_PROFILE_ID_29_account_tx_lookup\n"
    ".set __ZISKOS_PROFILE_ID_29_account_tx_lookup, 29\n"
    ".globl __ZISKOS_PROFILE_ID_30_account_block_lookup\n"
    ".set __ZISKOS_PROFILE_ID_30_account_block_lookup, 30\n"
    ".globl __ZISKOS_PROFILE_ID_31_account_witness\n"
    ".set __ZISKOS_PROFILE_ID_31_account_witness, 31\n"
    ".globl __ZISKOS_PROFILE_ID_32_storage_load\n"
    ".set __ZISKOS_PROFILE_ID_32_storage_load, 32\n"
    ".globl __ZISKOS_PROFILE_ID_33_storage_tx_lookup\n"
    ".set __ZISKOS_PROFILE_ID_33_storage_tx_lookup, 33\n"
    ".globl __ZISKOS_PROFILE_ID_34_storage_block_lookup\n"
    ".set __ZISKOS_PROFILE_ID_34_storage_block_lookup, 34\n"
    ".globl __ZISKOS_PROFILE_ID_35_storage_witness\n"
    ".set __ZISKOS_PROFILE_ID_35_storage_witness, 35\n"
    ".globl __ZISKOS_PROFILE_ID_36_bal_account_touch\n"
    ".set __ZISKOS_PROFILE_ID_36_bal_account_touch, 36\n"
    ".globl __ZISKOS_PROFILE_ID_37_bal_storage_read\n"
    ".set __ZISKOS_PROFILE_ID_37_bal_storage_read, 37\n"
    ".globl __ZISKOS_PROFILE_ID_38_index_witness_nodes\n"
    ".set __ZISKOS_PROFILE_ID_38_index_witness_nodes, 38\n"
    ".globl __ZISKOS_PROFILE_ID_39_index_witness_codes\n"
    ".set __ZISKOS_PROFILE_ID_39_index_witness_codes, 39\n"
    ".globl __ZISKOS_PROFILE_ID_40_index_witness_headers\n"
    ".set __ZISKOS_PROFILE_ID_40_index_witness_headers, 40\n"
    ".globl __ZISKOS_PROFILE_ID_41_request_withdrawals\n"
    ".set __ZISKOS_PROFILE_ID_41_request_withdrawals, 41\n"
    ".globl __ZISKOS_PROFILE_ID_42_request_consolidations\n"
    ".set __ZISKOS_PROFILE_ID_42_request_consolidations, 42\n"
    ".globl __ZISKOS_PROFILE_ID_43_request_builder_deposits\n"
    ".set __ZISKOS_PROFILE_ID_43_request_builder_deposits, 43\n"
    ".globl __ZISKOS_PROFILE_ID_44_request_builder_exits\n"
    ".set __ZISKOS_PROFILE_ID_44_request_builder_exits, 44\n"
    ".globl __ZISKOS_PROFILE_ID_45_account_mutation\n"
    ".set __ZISKOS_PROFILE_ID_45_account_mutation, 45\n"
    ".globl __ZISKOS_PROFILE_ID_46_storage_mutation\n"
    ".set __ZISKOS_PROFILE_ID_46_storage_mutation, 46\n"
    ".globl __ZISKOS_PROFILE_ID_47_tx_merge\n"
    ".set __ZISKOS_PROFILE_ID_47_tx_merge, 47\n"
    ".globl __ZISKOS_PROFILE_ID_48_tx_merge_accounts\n"
    ".set __ZISKOS_PROFILE_ID_48_tx_merge_accounts, 48\n"
    ".globl __ZISKOS_PROFILE_ID_49_tx_merge_storage\n"
    ".set __ZISKOS_PROFILE_ID_49_tx_merge_storage, 49\n"
    ".globl __ZISKOS_PROFILE_ID_50_system_call_interpret\n"
    ".set __ZISKOS_PROFILE_ID_50_system_call_interpret, 50\n"
    ".globl __ZISKOS_PROFILE_ID_51_system_call_merge\n"
    ".set __ZISKOS_PROFILE_ID_51_system_call_merge, 51\n"
);

#define START_SCOPE(id, tag) case id: __asm__ volatile("addi x0, x1, " #id ::: "memory"); break
#define END_SCOPE(id, tag) case id: __asm__ volatile("addi x0, x2, " #id ::: "memory"); break
#endif

unit evmsail_cycle_scope_start(uint64_t scope)
{
#if defined(__riscv)
    switch (scope) {
    START_SCOPE(0, stateless_validation);
    START_SCOPE(1, decode_input);
    START_SCOPE(2, index_witness);
    START_SCOPE(3, validate_payload);
    START_SCOPE(4, execute_block);
    START_SCOPE(5, validate_result);
    START_SCOPE(6, compute_output_root);
    START_SCOPE(7, serialize_output);
    START_SCOPE(8, block_start);
    START_SCOPE(9, block_transactions);
    START_SCOPE(10, tx_decode);
    START_SCOPE(11, tx_reset);
    START_SCOPE(12, tx_validate);
    START_SCOPE(13, tx_upfront);
    START_SCOPE(14, tx_frame);
    START_SCOPE(15, tx_settle);
    START_SCOPE(16, receipts_root);
    START_SCOPE(17, block_end_state);
    START_SCOPE(18, block_end_requests);
    START_SCOPE(19, state_root);
    START_SCOPE(20, block_access_list);
    START_SCOPE(21, htr_execution_payload);
    START_SCOPE(22, htr_transactions);
    START_SCOPE(23, htr_withdrawals);
    START_SCOPE(24, htr_versioned_hashes);
    START_SCOPE(25, htr_execution_requests);
    START_SCOPE(26, htr_bytes_root);
    START_SCOPE(27, htr_merkle_padding);
    START_SCOPE(28, account_load);
    START_SCOPE(29, account_tx_lookup);
    START_SCOPE(30, account_block_lookup);
    START_SCOPE(31, account_witness);
    START_SCOPE(32, storage_load);
    START_SCOPE(33, storage_tx_lookup);
    START_SCOPE(34, storage_block_lookup);
    START_SCOPE(35, storage_witness);
    START_SCOPE(36, bal_account_touch);
    START_SCOPE(37, bal_storage_read);
    START_SCOPE(38, index_witness_nodes);
    START_SCOPE(39, index_witness_codes);
    START_SCOPE(40, index_witness_headers);
    START_SCOPE(41, request_withdrawals);
    START_SCOPE(42, request_consolidations);
    START_SCOPE(43, request_builder_deposits);
    START_SCOPE(44, request_builder_exits);
    START_SCOPE(45, account_mutation);
    START_SCOPE(46, storage_mutation);
    START_SCOPE(47, tx_merge);
    START_SCOPE(48, tx_merge_accounts);
    START_SCOPE(49, tx_merge_storage);
    START_SCOPE(50, system_call_interpret);
    START_SCOPE(51, system_call_merge);
    default: break;
    }
#else
    (void)scope;
#endif
    return UNIT;
}

unit evmsail_cycle_scope_end(uint64_t scope)
{
#if defined(__riscv)
    switch (scope) {
    END_SCOPE(0, stateless_validation);
    END_SCOPE(1, decode_input);
    END_SCOPE(2, index_witness);
    END_SCOPE(3, validate_payload);
    END_SCOPE(4, execute_block);
    END_SCOPE(5, validate_result);
    END_SCOPE(6, compute_output_root);
    END_SCOPE(7, serialize_output);
    END_SCOPE(8, block_start);
    END_SCOPE(9, block_transactions);
    END_SCOPE(10, tx_decode);
    END_SCOPE(11, tx_reset);
    END_SCOPE(12, tx_validate);
    END_SCOPE(13, tx_upfront);
    END_SCOPE(14, tx_frame);
    END_SCOPE(15, tx_settle);
    END_SCOPE(16, receipts_root);
    END_SCOPE(17, block_end_state);
    END_SCOPE(18, block_end_requests);
    END_SCOPE(19, state_root);
    END_SCOPE(20, block_access_list);
    END_SCOPE(21, htr_execution_payload);
    END_SCOPE(22, htr_transactions);
    END_SCOPE(23, htr_withdrawals);
    END_SCOPE(24, htr_versioned_hashes);
    END_SCOPE(25, htr_execution_requests);
    END_SCOPE(26, htr_bytes_root);
    END_SCOPE(27, htr_merkle_padding);
    END_SCOPE(28, account_load);
    END_SCOPE(29, account_tx_lookup);
    END_SCOPE(30, account_block_lookup);
    END_SCOPE(31, account_witness);
    END_SCOPE(32, storage_load);
    END_SCOPE(33, storage_tx_lookup);
    END_SCOPE(34, storage_block_lookup);
    END_SCOPE(35, storage_witness);
    END_SCOPE(36, bal_account_touch);
    END_SCOPE(37, bal_storage_read);
    END_SCOPE(38, index_witness_nodes);
    END_SCOPE(39, index_witness_codes);
    END_SCOPE(40, index_witness_headers);
    END_SCOPE(41, request_withdrawals);
    END_SCOPE(42, request_consolidations);
    END_SCOPE(43, request_builder_deposits);
    END_SCOPE(44, request_builder_exits);
    END_SCOPE(45, account_mutation);
    END_SCOPE(46, storage_mutation);
    END_SCOPE(47, tx_merge);
    END_SCOPE(48, tx_merge_accounts);
    END_SCOPE(49, tx_merge_storage);
    END_SCOPE(50, system_call_interpret);
    END_SCOPE(51, system_call_merge);
    default: break;
    }
#else
    (void)scope;
#endif
    return UNIT;
}
