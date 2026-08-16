/* Native-only post-run diagnostics.
 *
 * Production modules expose ordinary invariant-preserving views; this file
 * performs the optional serialization directly and maintains no shadow guest
 * state. It is never part of the optimized guest source manifest. */
#include "evmsail/exceptions.h"
#include "evmsail/host/memory.h"
#include "evmsail/host/stack.h"
#include "evmsail/prelude.h"
#include "evmsail/spec/evm/machine.h"
#include "evmsail/spec/host/stack.h"
#include "host/state/internal.h"
#include "lib/mpt/trie.h"
#include "primitives/value.h"
#include "test/native.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static unsigned char dump_bytes[1u << 22];
static size_t dump_len;

static void append_byte(unsigned char byte)
{
  if (dump_len < sizeof dump_bytes)
    dump_bytes[dump_len++] = byte;
}

static void append_u32(uint32_t value)
{
  for (int index = 3; index >= 0; index--)
    append_byte((unsigned char)(value >> (8 * index)));
}

static void append_u64(uint64_t value)
{
  for (int index = 7; index >= 0; index--)
    append_byte((unsigned char)(value >> (8 * index)));
}

static void append_word(u256 value)
{
  uint64_t words[4];
  sail_word_to_be_words4(words, value);
  for (size_t index = 0; index < 4; index++) {
    const uint64_t limb = words[index];
    for (int byte = 0; byte < 8; byte++)
      append_byte((unsigned char)(limb >> (56 - 8 * byte)));
  }
}

static void append_address(bytes20 value)
{
  uint8_t bytes[20];
  address_to_be_bytes(bytes, value);
  for (size_t index = 0; index < sizeof bytes; index++)
    append_byte(bytes[index]);
}

static void append_hash(bytes32 value)
{
  const uint8_t *bytes = bytes32_data(&value);
  for (size_t index = 0; index < 32; index++)
    append_byte(bytes[index]);
}

static void append_location(const char *location)
{
  size_t len = 0;
  while (location[len] && len < 512)
    len++;
  append_byte((unsigned char)(len >> 8));
  append_byte((unsigned char)len);
  for (size_t index = 0; index < len; index++)
    append_byte((unsigned char)location[index]);
}

static uint32_t account_count(uint32_t binding_count)
{
  uint32_t count = 0;
  for (uint32_t index = 0; index < binding_count; index++) {
    AccountTrieView view;
    if (account_trie_binding_get(index, &view) && account_exists(view.account_id))
      count++;
  }
  return count;
}

static void append_storage(const AccountTrieView *account)
{
  uint32_t count = 0;
  for (uint32_t offset = 0; offset < account->storage_count; offset++) {
    StorageTrieView view;
    if (storage_trie_binding_get(account->storage_begin + offset, account->storage_generation,
                                 &view))
      count++;
  }
  append_u32(count);
  for (uint32_t offset = 0; offset < account->storage_count; offset++) {
    StorageTrieView view;
    const StorageId storage_id = account->storage_begin + offset;
    if (!storage_trie_binding_get(storage_id, account->storage_generation, &view))
      continue;
    append_word(*storage_id_slot(storage_id));
    append_word(*view.current);
  }
}

static void append_accounts(bool state_available)
{
  const uint32_t binding_count = state_available ? mpt_account_updates_prepare() : 0;
  append_u32(account_count(binding_count));
  for (uint32_t index = 0; index < binding_count; index++) {
    AccountTrieView view;
    if (!account_trie_binding_get(index, &view) || !account_exists(view.account_id))
      continue;
    const bytes20 address = *account_id_address(view.account_id);
    const bytes32 storage_root = *view.original_storage_root;
    append_hash(*view.secure_key);
    append_address(address);
    append_u64(view.nonce);
    append_word(*view.balance);
    append_hash(storage_root);
    append_hash(storage_root);
    append_hash(*view.code_hash);
    append_storage(&view);
  }
}

unsigned long guest_debug_dump(const unsigned char **out)
{
  dump_len = 0;
  bytes32 root = {{0}};
  const bool validation_failed = validation_failure_present;
  bool state_available = !validation_failed;
  if (state_available) {
    root = compute_state_root();
  }

  append_byte('G');
  append_byte(state_available ? 1 : 0);
  if (state_available) {
    append_hash(root);
  } else {
    for (size_t index = 0; index < 32; index++)
      append_byte(0);
    append_byte(validation_failed ? (unsigned char)validation_failure_reason : 0xff);
    append_location("");
  }

  size_t output_len;
  const uint8_t *output = native_output_data(&output_len);
  append_byte('O');
  append_u32((uint32_t)output_len);
  for (size_t index = 0; index < output_len; index++)
    append_byte(output[index]);

  append_byte('V');
  append_byte(validation_failure_present ? 1 : 0);
  if (validation_failure_present) {
    append_byte((unsigned char)validation_failure_scope);
    append_byte((unsigned char)validation_failure_reason);
    append_location("");
  }

  append_byte('B');
  append_byte(validation_debug_gas_present ? 1 : 0);
  if (validation_debug_gas_present) {
    append_u64(validation_debug_header_gas_actual);
    append_u64(validation_debug_header_gas_expected);
    append_u64(validation_debug_execution_gas);
    append_u64(validation_debug_state_gas);
  }

  append_byte('A');
  append_accounts(state_available);

  append_byte('S');
  append_u32(0);

  append_byte('M');
  append_u32(0);

  append_byte('E');
  *out = dump_bytes;
  return (unsigned long)dump_len;
}
