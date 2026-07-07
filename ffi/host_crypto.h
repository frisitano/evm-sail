/* Direct crypto helpers for C-side implementation hooks.
 *
 * Sail treats Keccak/SHA as abstract byte-list operations. C helpers here are
 * for optimized pointer/length refinements such as witness-code insertion into
 * the code DB.
 */
#ifndef HOST_CRYPTO_H
#define HOST_CRYPTO_H

#include "sail.h"
#include <stdint.h>

enum evmsail_byte_source_kind {
  EVMSAIL_SOURCE_WITNESS = 1,
  EVMSAIL_SOURCE_MEMORY = 2,
  EVMSAIL_SOURCE_TX_INPUT = 3,
  EVMSAIL_SOURCE_ACTIVE_CODE = 4,
  EVMSAIL_SOURCE_TRIE_ARENA = 5,
};

void host_keccak256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_sha256_bytes(uint64_t out[4], const uint8_t *p, uint64_t len);
void host_keccak256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_sha256_lbits(lbits *rop, const uint8_t *p, uint64_t len);
void host_keccak_word(lbits *rop, const lbits w);
void host_keccak_address(lbits *rop, const lbits a);
void host_keccak_create2(lbits *rop, const lbits sender, const lbits salt,
                         const lbits init_hash);
void host_create_address(lbits *rop, const lbits sender, uint64_t nonce);
void host_block_header_hash(
    lbits *rop, const lbits parent_hash, const lbits fee_recipient,
    const lbits state_root, const lbits transactions_root,
    const lbits receipts_root, uint64_t bloom_off, uint64_t number,
    uint64_t gas_limit, uint64_t gas_used, uint64_t timestamp,
    uint64_t extra_off, uint64_t extra_len, const lbits prev_randao,
    const lbits base_fee, const lbits withdrawals_root, uint64_t blob_gas_used,
    uint64_t excess_blob_gas, const lbits parent_beacon_block_root,
    const lbits requests_hash, const lbits block_access_list_hash,
    uint64_t slot_number);
void host_tx_signing_hash(lbits *rop, uint64_t ttype, uint64_t apply_155,
                          const lbits chain_id, uint64_t src_kind, uint64_t off,
                          uint64_t len);
void host_auth_signing_hash(lbits *rop, const lbits chain_id, const lbits address,
                            uint64_t nonce);
void host_sha256_pair(lbits *rop, const lbits a, const lbits b);
void host_sha256_digests3(lbits *rop, uint64_t mask, const lbits d0, const lbits d1,
                          const lbits d2);
int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **p, uint64_t *resolved_len);
void host_source_keccak(lbits *rop, uint64_t source_kind, uint64_t off, uint64_t len);
void host_source_sha256_prefixed(lbits *rop, uint64_t prefix, uint64_t source_kind,
                                 uint64_t off, uint64_t len);

unit host_bytes_reset(const unit u);
unit host_bytes_push(uint64_t b);
unit host_bytes_push8(uint64_t w);
void host_bytes_keccak_finish(lbits *rop, const unit u);
void host_bytes_sha256_finish(lbits *rop, const unit u);

unit node_asm_reset(const unit u);
unit node_asm_push_ref(uint64_t kind, const lbits data, uint64_t len);
unit node_asm_push_path(const lbits nibbles, uint64_t cnt, uint64_t is_leaf);
uint64_t node_asm_finish_branch(const unit u);
uint64_t node_asm_finish(const unit u);
void node_asm_result_data(lbits *rop, const unit u);
void node_asm_result_hash(lbits *rop, const unit u);

#endif
