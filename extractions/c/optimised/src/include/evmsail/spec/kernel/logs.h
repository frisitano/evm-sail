#pragma once

#include "evmsail/spec/kernel/storage.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void k_log_topics(struct LogTopics topics);

void k_log_data(struct LogData data);

void k_log(bytes20 a, struct LogTopics topics, struct LogData data);

struct LogSeriesRef read_logs(void);

Bytes read_log_data(uint64_t index);

void k_emit_transfer_log(bytes20 src, bytes20 dst, u256 v);

extern const uint16_t LOGS_BLOOM_BYTE_LENGTH;



#ifdef __cplusplus
}
#endif
