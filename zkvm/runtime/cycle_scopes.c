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
    default: break;
    }
#else
    (void)scope;
#endif
    return UNIT;
}
