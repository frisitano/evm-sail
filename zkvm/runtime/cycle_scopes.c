#include "cycle_scopes.h"

#if defined(__riscv)
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
);

#define START_SCOPE(id) case id: __asm__ volatile("addi x0, x1, " #id ::: "memory"); break
#define END_SCOPE(id) case id: __asm__ volatile("addi x0, x2, " #id ::: "memory"); break
#endif

unit evmsail_cycle_scope_start(uint64_t scope)
{
#if defined(__riscv)
    switch (scope) {
    START_SCOPE(0);
    START_SCOPE(1);
    START_SCOPE(2);
    START_SCOPE(3);
    START_SCOPE(4);
    START_SCOPE(5);
    START_SCOPE(6);
    START_SCOPE(7);
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
    END_SCOPE(0);
    END_SCOPE(1);
    END_SCOPE(2);
    END_SCOPE(3);
    END_SCOPE(4);
    END_SCOPE(5);
    END_SCOPE(6);
    END_SCOPE(7);
    default: break;
    }
#else
    (void)scope;
#endif
    return UNIT;
}
