#pragma once

#include "evmsail/spec/executor/receipts.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void enter_system_call_frame(bytes20 tgt, struct CalldataSlice input);

void system_call(bytes20 tgt, bytes32 input);

Bytes system_call_checked(bytes20 tgt);

bool deposit_log_matches(uint64_t index);

Bytes authenticate_deposit_request(Bytes data, Bytes expected);

Bytes authenticate_deposit_logs(struct LogSeriesRef logs, Bytes expected);

void validate_request_stream(bytes20 tgt, Bytes expected);

void validate_execution_requests(struct StatelessInputRef input_ref);

extern const uint8_t SYSTEM_CALL_INPUT_LENGTH;


extern const uint16_t DEPOSIT_EVENT_DATA_LENGTH;


extern const uint8_t DEPOSIT_PUBKEY_HEAD;


extern const uint8_t DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD;


extern const uint8_t DEPOSIT_AMOUNT_HEAD;


extern const uint8_t DEPOSIT_SIGNATURE_HEAD;


extern const uint8_t DEPOSIT_INDEX_HEAD;


extern const uint8_t DEPOSIT_PUBKEY_LENGTH_WORD;


extern const uint8_t DEPOSIT_PUBKEY_DATA;


extern const uint8_t DEPOSIT_PUBKEY_LENGTH;


extern const uint16_t DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD;


extern const uint16_t DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA;


extern const uint8_t DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH;


extern const uint16_t DEPOSIT_AMOUNT_LENGTH_WORD;


extern const uint16_t DEPOSIT_AMOUNT_DATA;


extern const uint8_t DEPOSIT_AMOUNT_LENGTH;


extern const uint16_t DEPOSIT_SIGNATURE_LENGTH_WORD;


extern const uint16_t DEPOSIT_SIGNATURE_DATA;


extern const uint8_t DEPOSIT_SIGNATURE_LENGTH;


extern const uint16_t DEPOSIT_INDEX_LENGTH_WORD;


extern const uint16_t DEPOSIT_INDEX_DATA;


extern const uint8_t DEPOSIT_INDEX_LENGTH;


extern const uint8_t DEPOSIT_REQUEST_LENGTH;


extern const uint8_t DEPOSIT_REQUEST_PUBKEY;


extern const uint8_t DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS;


extern const uint8_t DEPOSIT_REQUEST_AMOUNT;


extern const uint8_t DEPOSIT_REQUEST_SIGNATURE;


extern const uint8_t DEPOSIT_REQUEST_INDEX;



#ifdef __cplusplus
}
#endif
