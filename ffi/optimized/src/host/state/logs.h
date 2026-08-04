/* C-backed LOG series and receipt-bloom construction.
 *
 * The implementation owns the ordered log records with their topic and data
 * arenas, plus the block-wide logs-bloom accumulator derived from them.
 * Each completed record has one inverse event in the shared state journal;
 * rollback therefore truncates records, topics, and data in lockstep. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_LOGS_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_LOGS_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Generated aggregate adapter implemented by the region-access layer. */
struct EvmMemorySliceFields;
unit log_add_data_memory(struct EvmMemorySliceFields data);

unit logs_reset(unit ignored);
unit logs_tx_reset(unit ignored);
void logs_revert_last(void);

unit log_begin(Address address);
unit log_add_topic(U256 topic);
unit log_add_data_bulk(const uint8_t *bytes, uint64_t length);
unit log_add_data_word(U256 value);

uint64_t log_count(unit ignored);
uint64_t logs_tx_start(unit ignored);
uint64_t logs_tx_count(unit ignored);
Address log_addr(uint64_t index);
uint64_t log_topic_count(uint64_t index);
U256 log_topic(uint64_t log_index, uint64_t topic_index);
uint32_t log_data_len(uint64_t index);
uint32_t log_data_off(uint64_t index);
const uint8_t *log_data_region(uint64_t offset, uint64_t length);
const uint8_t *log_data_base(void);

/* The receipt writer sets sparse bits directly in its final RLP span while
 * maintaining the block-wide 32-word accumulator. */
unit receipt_runtime_block_bloom_reset(unit ignored);
LogsBloom receipt_runtime_block_bloom(unit ignored);
bool receipt_runtime_bloom_write(uint64_t start, uint64_t count,
                                 uint8_t out[256]);

void state_logs_workspace_bind(void);

#endif
