#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint16_t LOGS_BLOOM_BYTE_LENGTH = UINT16_C(256);


void k_log_topics(struct LogTopics topics)
{
  if (topics.kind != Kind_LogTopics0) {
    switch (topics.kind) {
    case Kind_LogTopics1:
      log_add_topic(topics.variants.LogTopics1);
      return;
    case Kind_LogTopics2:
      log_add_topic(topics.variants.LogTopics2.tup0);
      log_add_topic(topics.variants.LogTopics2.tup1);
      return;
    case Kind_LogTopics3:
      log_add_topic(topics.variants.LogTopics3.tup0);
      log_add_topic(topics.variants.LogTopics3.tup1);
      log_add_topic(topics.variants.LogTopics3.tup2);
      return;
    case Kind_LogTopics0:
    case Kind_LogTopics4:
      log_add_topic(topics.variants.LogTopics4.tup0);
      log_add_topic(topics.variants.LogTopics4.tup1);
      log_add_topic(topics.variants.LogTopics4.tup2);
      log_add_topic(topics.variants.LogTopics4.tup3);
      return;
    }
  }
}

void k_log_data(struct LogData data)
{
  switch (data.kind) {
  case Kind_LogDataMemory:
    log_add_data_memory(data.variants.LogDataMemory);
    return;
  case Kind_LogDataWord:
    log_add_data_word(data.variants.LogDataWord);
    return;
  }
}

void k_log(bytes20 a, struct LogTopics topics, struct LogData data)
{
  log_begin(a);
  k_log_topics(topics);
  k_log_data(data);
}

struct LogSeriesRef read_logs(void)
{
  uint64_t start = logs_tx_start();
  uint64_t count = logs_tx_count();
  return ((struct LogSeriesRef){.count = count, .start = start});
}

Bytes read_log_data(uint64_t index)
{
  return host_log_data_slice(index);
}

void k_emit_transfer_log(bytes20 src, bytes20 dst, u256 v)
{
  if ((k_execution_profile.protocol.fork < Amsterdam) || (eq_u256(v, WORD_ZERO) || eq_bytes20(src, dst))) {
    return;
  }
  u256 source = address_to_word(src);
  u256 destination = address_to_word(dst);
  struct LogTopics topics = LogTopics3(((struct tuple_u256_u256_u256){.tup0 = EIP7708_TRANSFER_TOPIC, .tup1 = source, .tup2 = destination}));
  k_log(EIP7708_SYSTEM_ADDRESS, topics, (LogDataWord(v)));
}

