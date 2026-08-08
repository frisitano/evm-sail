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

void logs_reset(void);
void logs_revert_last(void);

void log_add_data_bulk(const uint8_t *bytes, uint64_t length);

uint64_t log_count(void);
uint32_t log_data_len(uint64_t index);
uint32_t log_data_off(uint64_t index);
const uint8_t *log_data_base(void);
uint32_t log_data_capacity(void);

/* The receipt writer sets sparse bits directly in its final RLP span while
 * maintaining the block-wide 32-word accumulator. */
void receipt_runtime_block_bloom_reset(void);
bool receipt_runtime_block_bloom_matches(const uint8_t *reference, uint64_t length);
bool receipt_runtime_bloom_write(uint64_t start, uint64_t count, uint8_t out[256]);

void state_logs_workspace_bind(uint32_t record_capacity, uint32_t topic_capacity,
                               uint32_t data_capacity);

#endif
