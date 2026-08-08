#pragma once

#include "evmsail/spec.h"

static bool eq_bytes20(bytes20 op1, bytes20 op2);
static bool eq_MemoryRangeFields(struct MemoryRangeFields op1, struct MemoryRangeFields op2);
static bool eq_Bytes(Bytes op1, Bytes op2);
static bool eq_u256(u256 op1, u256 op2);
static bool eq_ExceptionKind(enum ExceptionKind op1, enum ExceptionKind op2);
static bool eq_HaltKind(struct HaltKind op1, struct HaltKind op2);
static bool eq_BoundedSszListRef(struct BoundedSszListRef op1, struct BoundedSszListRef op2);
static bool eq_BoundedSszListCursor(struct BoundedSszListCursor op1, struct BoundedSszListCursor op2);
static bool eq_BlobScheduleFields(struct BlobScheduleFields op1, struct BlobScheduleFields op2);
static bool eq_GasLimitsFields(struct GasLimitsFields op1, struct GasLimitsFields op2);
static bool eq_ProtocolProfileFields(struct ProtocolProfileFields op1, struct ProtocolProfileFields op2);
static bool eq_StorageKey(struct StorageKey op1, struct StorageKey op2);
static bool eq_StorageValue(struct StorageValue op1, struct StorageValue op2);
static bool eq_bytes32(bytes32 op1, bytes32 op2);
static bool eq_StorageEntry(struct StorageEntry op1, struct StorageEntry op2);
static bool eq_StorageTrieEntry(struct StorageTrieEntry op1, struct StorageTrieEntry op2);
static bool eq_AccountInfo(struct AccountInfo op1, struct AccountInfo op2);
static bool eq_Account(struct Account op1, struct Account op2);
static bool eq_AcctValue(struct AcctValue op1, struct AcctValue op2);
static bool eq_AcctEntry(struct AcctEntry op1, struct AcctEntry op2);
static bool eq_AcctTrieEntry(struct AcctTrieEntry op1, struct AcctTrieEntry op2);
static bool eq_TxSignatureScheme(enum TxSignatureScheme op1, enum TxSignatureScheme op2);
static bool eq_tuple_u256_u256(struct tuple_u256_u256 op1, struct tuple_u256_u256 op2);
static bool eq_tuple_u256_u256_u256(struct tuple_u256_u256_u256 op1, struct tuple_u256_u256_u256 op2);
static bool eq_tuple_u256_u256_u256_u256(struct tuple_u256_u256_u256_u256 op1, struct tuple_u256_u256_u256_u256 op2);
static bool eq_LogSeriesRef(struct LogSeriesRef op1, struct LogSeriesRef op2);
static bool eq_TxType(enum TxType op1, enum TxType op2);
static bool eq_AccessListRef(struct AccessListRef op1, struct AccessListRef op2);
static bool eq_AuthorizationListRefFields(struct AuthorizationListRefFields op1, struct AuthorizationListRefFields op2);
static bool eq_BlobHashesFields(struct BlobHashesFields op1, struct BlobHashesFields op2);
static bool eq_BlockBody(struct BlockBody op1, struct BlockBody op2);
static bool eq_BlockHeader(struct BlockHeader op1, struct BlockHeader op2);
static bool eq_Block(struct Block op1, struct Block op2);
static bool eq_TxFrameGasSnapshotFields(struct TxFrameGasSnapshotFields op1, struct TxFrameGasSnapshotFields op2);
static bool eq_TransactionInitialGasFields(struct TransactionInitialGasFields op1, struct TransactionInitialGasFields op2);
static bool eq_CalldataSlice(struct CalldataSlice op1, struct CalldataSlice op2);
static bool eq_CodeFields(struct CodeFields op1, struct CodeFields op2);
static bool eq_Message(struct Message op1, struct Message op2);
static bool eq_FrameStatus(struct FrameStatus op1, struct FrameStatus op2);
static bool eq_FrameCheckpoint(struct FrameCheckpoint op1, struct FrameCheckpoint op2);
static bool eq_CallContinuation(struct CallContinuation op1, struct CallContinuation op2);
static bool eq_CreateContinuation(struct CreateContinuation op1, struct CreateContinuation op2);
static bool eq_ChainConfig(struct ChainConfig op1, struct ChainConfig op2);
static bool eq_ExecutionPayload(struct ExecutionPayload op1, struct ExecutionPayload op2);
static bool eq_tuple_uint_8_u256(struct tuple_uint_8_u256 op1, struct tuple_uint_8_u256 op2);
static bool eq_LogTopics(struct LogTopics op1, struct LogTopics op2);
static bool eq_TriePath(struct TriePath op1, struct TriePath op2);
static bool eq_InlineNode(struct InlineNode op1, struct InlineNode op2);
static bool eq_NodeRef(struct NodeRef op1, struct NodeRef op2);
static bool eq_vector_16_NodeRef(vector_16_NodeRef op1, vector_16_NodeRef op2);
static bool eq_tuple_vector_16_NodeRef_Bytes(struct tuple_vector_16_NodeRef_Bytes op1, struct tuple_vector_16_NodeRef_Bytes op2);
static bool eq_tuple_TriePath_NodeRef(struct tuple_TriePath_NodeRef op1, struct tuple_TriePath_NodeRef op2);
static bool eq_tuple_TriePath_Bytes(struct tuple_TriePath_Bytes op1, struct tuple_TriePath_Bytes op2);
static bool eq_tuple_vector_16_NodeRef_Bytes_1(struct tuple_vector_16_NodeRef_Bytes_1 op1, struct tuple_vector_16_NodeRef_Bytes_1 op2);
static bool eq_tuple_TriePath_Bytes_1(struct tuple_TriePath_Bytes_1 op1, struct tuple_TriePath_Bytes_1 op2);
static bool eq_TrieChange(struct TrieChange op1, struct TrieChange op2);
static bool eq_TrieUpdate(struct TrieUpdate op1, struct TrieUpdate op2);
static bool eq_TrieUpdateRelation(struct TrieUpdateRelation op1, struct TrieUpdateRelation op2);
static bool eq_TrieUpdateSource(struct TrieUpdateSource op1, struct TrieUpdateSource op2);
static bool eq_TrieLeafValue(struct TrieLeafValue op1, struct TrieLeafValue op2);
static bool eq_TrieItemValue(struct TrieItemValue op1, struct TrieItemValue op2);
static bool eq_TrieItem(struct TrieItem op1, struct TrieItem op2);
static bool eq_TrieUpdateCursor(struct TrieUpdateCursor op1, struct TrieUpdateCursor op2);
static bool eq_u128(u128 op1, u128 op2);


_Noreturn static inline void sail_native_conversion_failure(const char *operation) {
  const int write_status = fprintf(stderr, "Sail C backend: %s\n", operation);
  (void)write_status;
  exit(EXIT_FAILURE);
}



static inline u256 u256_zero(void) {
  u256 result = {{0}};
  return result;
}

static inline u256 u256_of_u128(const u128 value) {
  u256 result = {{value.limbs[0], value.limbs[1], UINT64_C(0), UINT64_C(0)}};
  return result;
}

static inline u128 u128_of_u256_unchecked(const u256 value) {
  u128 result = {{value.limbs[0], value.limbs[1]}};
  return result;
}

static inline u128 u128_of_u256(const u256 value) {
  if (value.limbs[2] != UINT64_C(0) || value.limbs[3] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint128_t domain");
  }
  return u128_of_u256_unchecked(value);
}

static inline uint64_t u256_to_u64_unchecked(const u256 value) {
  return value.limbs[0];
}

static inline uint64_t u256_to_u64(const u256 value) {
  if (value.limbs[1] != UINT64_C(0)
      || value.limbs[2] != UINT64_C(0)
      || value.limbs[3] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint64_t domain");
  }
  return u256_to_u64_unchecked(value);
}

static inline bool eq_u256(const u256 op1, const u256 op2) {
  return (bool)(op1.limbs[0] == op2.limbs[0]
      && op1.limbs[1] == op2.limbs[1]
      && op1.limbs[2] == op2.limbs[2]
      && op1.limbs[3] == op2.limbs[3]);
}

static inline uint64_t u256_bit(const u256 value, const int64_t index) {
  if (index < 0 || index >= 256) {
    return UINT64_C(0);
  }
  return (value.limbs[(uint64_t)index >> 6] >> ((uint64_t)index & UINT64_C(63))) & UINT64_C(1);
}

static inline uint64_t u256_extract_u64(const u256 value, const uint64_t start) {
  if (start >= UINT64_C(256)) {
    return UINT64_C(0);
  }
  const size_t limb = (size_t)(start >> 6);
  const unsigned offset = (unsigned)(start & UINT64_C(63));
  uint64_t result = value.limbs[limb] >> offset;
  if (offset != 0 && limb + 1 < 4) {
    result |= value.limbs[limb + 1] << (64 - offset);
  }
  return result;
}

static inline u256 u256_update_u64(u256 value, const uint64_t index,
                                        const uint64_t bit) {
  if (index >= UINT64_C(256)) {
    return value;
  }
  const size_t limb = (size_t)(index >> 6);
  const uint64_t mask = UINT64_C(1) << (index & UINT64_C(63));
  if ((bit & UINT64_C(1)) != 0) {
    value.limbs[limb] |= mask;
  } else {
    value.limbs[limb] &= ~mask;
  }
  return value;
}

static inline u256 u256_update_i64(u256 value, const int64_t index,
                                        const uint64_t bit) {
  if (index < INT64_C(0)) {
    return value;
  }
  return u256_update_u64(value, (uint64_t)index, bit);
}

static inline uint64_t fast_vector_access_u256(const u256 value, const int64_t index) {
  return u256_bit(value, index);
}

static inline u256 u256_not(const u256 value) {
  u256 result = {0};
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = ~value.limbs[i];
  }
  return result;
}

static inline u256 u256_and(const u256 lhs, const u256 rhs) {
  u256 result = {0};
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = lhs.limbs[i] & rhs.limbs[i];
  }
  return result;
}

static inline u256 u256_or(const u256 lhs, const u256 rhs) {
  u256 result = {0};
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = lhs.limbs[i] | rhs.limbs[i];
  }
  return result;
}

static inline u256 u256_xor(const u256 lhs, const u256 rhs) {
  u256 result = {0};
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = lhs.limbs[i] ^ rhs.limbs[i];
  }
  return result;
}

static inline u256 u256_add(const u256 lhs, const u256 rhs) {
  u256 result = {0};
  uint64_t carry = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const uint64_t partial = lhs.limbs[i] + rhs.limbs[i];
    const uint64_t carry1 = partial < lhs.limbs[i];
    result.limbs[i] = partial + carry;
    const uint64_t carry2 = result.limbs[i] < partial;
    carry = carry1 | carry2;
  }
  return result;
}

static inline u256 u256_add_u128_u128(const u128 lhs,
                                           const u128 rhs) {
  u256 result = {{0}};
  const unsigned __int128 low =
      (unsigned __int128)lhs.limbs[0] + rhs.limbs[0];
  result.limbs[0] = (uint64_t)low;
  const unsigned __int128 high =
      (unsigned __int128)lhs.limbs[1] + rhs.limbs[1] + (low >> 64);
  result.limbs[1] = (uint64_t)high;
  result.limbs[2] = (uint64_t)(high >> 64);
  return result;
}

static inline u256 u256_add_u128_u64(const u128 lhs,
                                          const uint64_t rhs) {
  u256 result = {{0}};
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] + rhs;
  result.limbs[0] = (uint64_t)low;
  const unsigned __int128 high = (unsigned __int128)lhs.limbs[1] + (low >> 64);
  result.limbs[1] = (uint64_t)high;
  result.limbs[2] = (uint64_t)(high >> 64);
  return result;
}

static inline u256 u256_add_u64_u128(const uint64_t lhs,
                                          const u128 rhs) {
  return u256_add_u128_u64(rhs, lhs);
}

static inline u256 u256_sub(const u256 lhs, const u256 rhs) {
  u256 result = {0};
  uint64_t borrow = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const uint64_t partial = lhs.limbs[i] - rhs.limbs[i];
    const uint64_t borrow1 = lhs.limbs[i] < rhs.limbs[i];
    result.limbs[i] = partial - borrow;
    const uint64_t borrow2 = partial < borrow;
    borrow = borrow1 | borrow2;
  }
  return result;
}

static inline u256 u256_mul(const u256 lhs, const u256 rhs) {
  u256 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; i + j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 product = (unsigned __int128)lhs.limbs[i] * rhs.limbs[j];
      const unsigned __int128 sum = product + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
  }
  return result;
}

static inline u256 u256_mul_u128_u128(const u128 lhs,
                                           const u128 rhs) {
  u256 result = {{0}};
  for (size_t i = 0; i < 2; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; j < 2; ++j) {
      const size_t k = i + j;
      const unsigned __int128 sum =
          ((unsigned __int128)lhs.limbs[i] * rhs.limbs[j])
          + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
    result.limbs[i + 2] = (uint64_t)carry;
  }
  return result;
}

static inline u256 u256_mul_u128_u64(const u128 lhs,
                                          const uint64_t rhs) {
  u256 result = {{0}};
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < 2; ++i) {
    const unsigned __int128 product =
        ((unsigned __int128)lhs.limbs[i] * rhs) + carry;
    result.limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  result.limbs[2] = (uint64_t)carry;
  return result;
}

static inline u256 u256_mul_u64_u128(const uint64_t lhs,
                                          const u128 rhs) {
  return u256_mul_u128_u64(rhs, lhs);
}

static inline bool u256_is_zero(const u256 value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] | value.limbs[3])
      == UINT64_C(0);
}

static inline bool u256_lt(const u256 lhs, const u256 rhs) {
  for (size_t i = 4; i-- > 0;) {
    if (lhs.limbs[i] != rhs.limbs[i]) {
      return lhs.limbs[i] < rhs.limbs[i];
    }
  }
  return false;
}

static inline bool u256_eq_u128(const u256 lhs, const u128 rhs) {
  return (bool)(lhs.limbs[0] == rhs.limbs[0]
      && lhs.limbs[1] == rhs.limbs[1]
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0));
}

static inline bool u256_lt_u128(const u256 lhs, const u128 rhs) {
  if ((lhs.limbs[2] | lhs.limbs[3]) != UINT64_C(0)) {
    return false;
  }
  if (lhs.limbs[1] != rhs.limbs[1]) {
    return lhs.limbs[1] < rhs.limbs[1];
  }
  return lhs.limbs[0] < rhs.limbs[0];
}

static inline bool u128_lt_u256(const u128 lhs, const u256 rhs) {
  if ((rhs.limbs[2] | rhs.limbs[3]) != UINT64_C(0)) {
    return true;
  }
  if (lhs.limbs[1] != rhs.limbs[1]) {
    return lhs.limbs[1] < rhs.limbs[1];
  }
  return lhs.limbs[0] < rhs.limbs[0];
}

static inline bool u256_eq_u64(const u256 lhs, const uint64_t rhs) {
  return (bool)(lhs.limbs[0] == rhs
      && lhs.limbs[1] == UINT64_C(0)
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0));
}

static inline bool u256_lt_u64(const u256 lhs, const uint64_t rhs) {
  return (bool)(lhs.limbs[1] == UINT64_C(0)
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0)
      && lhs.limbs[0] < rhs);
}

static inline bool u64_lt_u256(const uint64_t lhs, const u256 rhs) {
  return (bool)(rhs.limbs[1] != UINT64_C(0)
      || rhs.limbs[2] != UINT64_C(0)
      || rhs.limbs[3] != UINT64_C(0)
      || lhs < rhs.limbs[0]);
}

static inline u256 u256_add_u64(const u256 lhs,
                                     const uint64_t rhs) {
  u256 result = lhs;
  result.limbs[0] += rhs;
  uint64_t carry = result.limbs[0] < lhs.limbs[0];
  for (size_t i = 1; i < 4 && carry != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]++;
    carry = result.limbs[i] < previous;
  }
  return result;
}

static inline u256 u256_add_u128(const u256 lhs,
                                      const u128 rhs) {
  u256 result = lhs;
  unsigned __int128 sum =
      (unsigned __int128)lhs.limbs[0] + rhs.limbs[0];
  result.limbs[0] = (uint64_t)sum;
  sum = (unsigned __int128)lhs.limbs[1] + rhs.limbs[1] + (sum >> 64);
  result.limbs[1] = (uint64_t)sum;
  uint64_t carry = (uint64_t)(sum >> 64);
  for (size_t i = 2; i < 4 && carry != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]++;
    carry = result.limbs[i] < previous;
  }
  return result;
}

static inline u256 u256_sub_u64(const u256 lhs,
                                     const uint64_t rhs) {
  u256 result = lhs;
  result.limbs[0] -= rhs;
  uint64_t borrow = lhs.limbs[0] < rhs;
  for (size_t i = 1; i < 4 && borrow != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]--;
    borrow = previous == UINT64_C(0);
  }
  return result;
}

static inline u256 u256_sub_u128(const u256 lhs,
                                      const u128 rhs) {
  u256 result = lhs;
  const uint64_t low = lhs.limbs[0] - rhs.limbs[0];
  uint64_t borrow = lhs.limbs[0] < rhs.limbs[0];
  const uint64_t high = lhs.limbs[1] - rhs.limbs[1];
  const uint64_t borrow1 = lhs.limbs[1] < rhs.limbs[1];
  result.limbs[0] = low;
  result.limbs[1] = high - borrow;
  const uint64_t borrow2 = high < borrow;
  borrow = borrow1 | borrow2;
  for (size_t i = 2; i < 4 && borrow != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]--;
    borrow = previous == UINT64_C(0);
  }
  return result;
}

/* A valid natural subtraction with a u128 minuend proves that the u256
 * subtrahend's upper limbs are zero. */
static inline u128 u128_sub_u256(const u128 lhs,
                                      const u256 rhs) {
  u128 result;
  result.limbs[0] = lhs.limbs[0] - rhs.limbs[0];
  const uint64_t borrow = lhs.limbs[0] < rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] - rhs.limbs[1] - borrow;
  return result;
}

/* The Sail range checker proves that rhs fits and rhs <= lhs at each emitted
 * u64 - u256 call site.  The helper only expresses the selected C
 * representation; it does not add a second runtime semantics. */
static inline uint64_t u64_sub_u256(const uint64_t lhs,
                                    const u256 rhs) {
  return lhs - rhs.limbs[0];
}

static inline u256 u256_mul_u64(const u256 lhs,
                                     const uint64_t rhs) {
  u256 result = {{0}};
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < 4; ++i) {
    const unsigned __int128 product =
        ((unsigned __int128)lhs.limbs[i] * rhs) + carry;
    result.limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  return result;
}

static inline u256 u256_mul_u128(const u256 lhs,
                                      const u128 rhs) {
  u256 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; j < 2 && i + j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 sum =
          ((unsigned __int128)lhs.limbs[i] * rhs.limbs[j])
          + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
    if (i + 2 < 4) {
      result.limbs[i + 2] = (uint64_t)carry;
    }
  }
  return result;
}

static inline u256 u256_div_u64(const u256 dividend,
                                     const uint64_t divisor) {
  u256 quotient = {{0}};
  uint64_t remainder = UINT64_C(0);
  for (size_t i = 4; i-- > 0;) {
    const unsigned __int128 partial =
        ((unsigned __int128)remainder << 64) | dividend.limbs[i];
    quotient.limbs[i] = (uint64_t)(partial / divisor);
    remainder = (uint64_t)(partial % divisor);
  }
  return quotient;
}

static inline u256 u256_mod_u64(const u256 dividend,
                                     const uint64_t divisor) {
  u256 result = {{0}};
  uint64_t remainder = UINT64_C(0);
  for (size_t i = 4; i-- > 0;) {
    const unsigned __int128 partial =
        ((unsigned __int128)remainder << 64) | dividend.limbs[i];
    remainder = (uint64_t)(partial % divisor);
  }
  result.limbs[0] = remainder;
  return result;
}

static inline size_t u256_significant_words(const uint64_t *value,
                                            size_t count) {
  while (count != 0 && value[count - 1] == UINT64_C(0)) {
    count--;
  }
  return count;
}

static inline unsigned u256_leading_zeros(const uint64_t value) {
  return value == UINT64_C(0) ? 64U : (unsigned)__builtin_clzll(value);
}

/* Knuth division, Algorithm D, in base 2^64.  The numerator has at most
 * eight limbs and the divisor at most four.  Callers either carry the JIB
 * nonzero proof or implement an explicit source-level zero-modulus branch. */
static inline void u256_divrem_words(const uint64_t *numerator,
                                     size_t numerator_count,
                                     const uint64_t divisor[4],
                                     uint64_t quotient[8],
                                     uint64_t remainder[4]) {
  uint64_t u[9] = {0};
  uint64_t v[4] = {0};
  const size_t un = u256_significant_words(numerator, numerator_count);
  const size_t vn = u256_significant_words(divisor, 4);

  if (quotient != NULL) {
    for (size_t i = 0; i < 8; ++i) {
      quotient[i] = UINT64_C(0);
    }
  }
  if (remainder != NULL) {
    for (size_t i = 0; i < 4; ++i) {
      remainder[i] = UINT64_C(0);
    }
  }
  if (un == 0) {
    return;
  }
  if (un < vn) {
    if (remainder != NULL) {
      for (size_t i = 0; i < un; ++i) {
        remainder[i] = numerator[i];
      }
    }
    return;
  }

  if (vn == 1) {
    uint64_t rem = UINT64_C(0);
    for (size_t i = un; i-- > 0;) {
      const unsigned __int128 partial =
          ((unsigned __int128)rem << 64) | numerator[i];
      if (quotient != NULL) {
        quotient[i] = (uint64_t)(partial / divisor[0]);
      }
      rem = (uint64_t)(partial % divisor[0]);
    }
    if (remainder != NULL) {
      remainder[0] = rem;
    }
    return;
  }

  const unsigned shift = u256_leading_zeros(divisor[vn - 1]);
  if (shift == 0) {
    for (size_t i = 0; i < vn; ++i) {
      v[i] = divisor[i];
    }
    for (size_t i = 0; i < un; ++i) {
      u[i] = numerator[i];
    }
  } else {
    uint64_t carry = UINT64_C(0);
    for (size_t i = 0; i < vn; ++i) {
      const uint64_t next = divisor[i] >> (64 - shift);
      v[i] = (divisor[i] << shift) | carry;
      carry = next;
    }
    carry = UINT64_C(0);
    for (size_t i = 0; i < un; ++i) {
      const uint64_t next = numerator[i] >> (64 - shift);
      u[i] = (numerator[i] << shift) | carry;
      carry = next;
    }
    u[un] = carry;
  }

  const size_t qn = un - vn + 1;
  for (size_t jj = qn; jj-- > 0;) {
    const size_t j = jj;
    uint64_t qhat;
    uint64_t rhat;
    bool rhat_overflow = false;
    if (u[j + vn] == v[vn - 1]) {
      qhat = UINT64_MAX;
      rhat = u[j + vn - 1] + v[vn - 1];
      rhat_overflow = rhat < u[j + vn - 1];
    } else {
      const unsigned __int128 top =
          ((unsigned __int128)u[j + vn] << 64) | u[j + vn - 1];
      qhat = (uint64_t)(top / v[vn - 1]);
      rhat = (uint64_t)(top % v[vn - 1]);
    }

    while (!rhat_overflow
           && (unsigned __int128)qhat * v[vn - 2]
                  > (((unsigned __int128)rhat << 64) | u[j + vn - 2])) {
      qhat--;
      const uint64_t next = rhat + v[vn - 1];
      rhat_overflow = next < rhat;
      rhat = next;
    }

    uint64_t borrow = UINT64_C(0);
    for (size_t i = 0; i < vn; ++i) {
      const unsigned __int128 product =
          ((unsigned __int128)qhat * v[i]) + borrow;
      const uint64_t low = (uint64_t)product;
      borrow = (uint64_t)(product >> 64) + (u[j + i] < low);
      u[j + i] -= low;
    }
    const bool negative = u[j + vn] < borrow;
    u[j + vn] -= borrow;

    if (negative) {
      qhat--;
      uint64_t carry = UINT64_C(0);
      for (size_t i = 0; i < vn; ++i) {
        const unsigned __int128 sum =
            (unsigned __int128)u[j + i] + v[i] + carry;
        u[j + i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
      }
      u[j + vn] += carry;
    }
    if (quotient != NULL) {
      quotient[j] = qhat;
    }
  }

  if (remainder != NULL) {
    if (shift == 0) {
      for (size_t i = 0; i < vn; ++i) {
        remainder[i] = u[i];
      }
    } else {
      for (size_t i = 0; i < vn; ++i) {
        remainder[i] = u[i] >> shift;
        if (i + 1 < vn) {
          remainder[i] |= u[i + 1] << (64 - shift);
        }
      }
    }
  }
}

static inline u256 u256_div_u128(const u256 dividend,
                                      const u128 divisor) {
  u256 result = {{0}};
  const uint64_t words[4] = {
      divisor.limbs[0], divisor.limbs[1], UINT64_C(0), UINT64_C(0)};
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 4, words, quotient, NULL);
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = quotient[i];
  }
  return result;
}

static inline u256 u256_mod_u128(const u256 dividend,
                                      const u128 divisor) {
  u256 result = {{0}};
  const uint64_t words[4] = {
      divisor.limbs[0], divisor.limbs[1], UINT64_C(0), UINT64_C(0)};
  u256_divrem_words(dividend.limbs, 4, words, NULL, result.limbs);
  return result;
}

static inline u128 u128_div_u256(const u128 dividend,
                                      const u256 divisor) {
  u128 result = {{0}};
  if ((divisor.limbs[2] | divisor.limbs[3]) != UINT64_C(0)) {
    return result;
  }
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 2, divisor.limbs, quotient, NULL);
  result.limbs[0] = quotient[0];
  result.limbs[1] = quotient[1];
  return result;
}

static inline u128 u128_mod_u256(const u128 dividend,
                                      const u256 divisor) {
  if ((divisor.limbs[2] | divisor.limbs[3]) != UINT64_C(0)) {
    return dividend;
  }
  uint64_t remainder[4] = {0};
  u256_divrem_words(dividend.limbs, 2, divisor.limbs, NULL, remainder);
  u128 result = {{remainder[0], remainder[1]}};
  return result;
}

static inline u256 u256_div(const u256 dividend,
                                 const u256 divisor) {
  u256 result = {{0}};
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 4, divisor.limbs, quotient, NULL);
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = quotient[i];
  }
  return result;
}

static inline u256 u256_mod(const u256 dividend,
                                 const u256 divisor) {
  u256 result = {{0}};
  u256_divrem_words(dividend.limbs, 4, divisor.limbs, NULL, result.limbs);
  return result;
}

static inline u256 u256_addmod(const u256 lhs,
                                    const u256 rhs,
                                    const u256 modulus) {
  u256 result = {{0}};
  uint64_t sum[5] = {0};
  uint64_t carry = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const unsigned __int128 wide =
        (unsigned __int128)lhs.limbs[i] + rhs.limbs[i] + carry;
    sum[i] = (uint64_t)wide;
    carry = (uint64_t)(wide >> 64);
  }
  sum[4] = carry;
  if (!u256_is_zero(modulus)) {
    u256_divrem_words(sum, 5, modulus.limbs, NULL, result.limbs);
  }
  return result;
}

static inline u256 u256_mulmod(const u256 lhs,
                                    const u256 rhs,
                                    const u256 modulus) {
  u256 result = {{0}};
  uint64_t product[8] = {0};
  for (size_t i = 0; i < 4; ++i) {
    uint64_t carry = UINT64_C(0);
    for (size_t j = 0; j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 wide =
          ((unsigned __int128)lhs.limbs[i] * rhs.limbs[j])
          + product[k] + carry;
      product[k] = (uint64_t)wide;
      carry = (uint64_t)(wide >> 64);
    }
    product[i + 4] = carry;
  }
  if (!u256_is_zero(modulus)) {
    u256_divrem_words(product, 8, modulus.limbs, NULL, result.limbs);
  }
  return result;
}

static inline u256 u256_shiftl_u64(const u256 value, const uint64_t amount) {
  u256 result = {{0}};
  if (amount >= UINT64_C(256)) {
    return result;
  }
  const size_t words = (size_t)(amount >> 6);
  const unsigned bits = (unsigned)(amount & UINT64_C(63));
  for (size_t dst = 4; dst-- > words;) {
    result.limbs[dst] = value.limbs[dst - words] << bits;
    if (bits != 0 && dst > words) {
      result.limbs[dst] |= value.limbs[dst - words - 1] >> (64 - bits);
    }
  }
  return result;
}

static inline u256 u256_shiftr_u64(const u256 value, const uint64_t amount) {
  u256 result = {{0}};
  if (amount >= UINT64_C(256)) {
    return result;
  }
  const size_t words = (size_t)(amount >> 6);
  const unsigned bits = (unsigned)(amount & UINT64_C(63));
  for (size_t dst = 0; dst + words < 4; ++dst) {
    result.limbs[dst] = value.limbs[dst + words] >> bits;
    if (bits != 0 && dst + words + 1 < 4) {
      result.limbs[dst] |= value.limbs[dst + words + 1] << (64 - bits);
    }
  }
  return result;
}

static inline u256 u256_arith_shiftr_u64(const u256 value, const uint64_t amount) {
  if ((value.limbs[3] >> 63) == 0) {
    return u256_shiftr_u64(value, amount);
  }
  return u256_not(u256_shiftr_u64(u256_not(value), amount));
}

static inline uint64_t u256_abs_i64(const int64_t amount) {
  return amount < 0 ? (uint64_t)(-(amount + 1)) + UINT64_C(1) : (uint64_t)amount;
}

static inline u256 u256_shiftl_i64(const u256 value, const int64_t amount) {
  return u256_shiftl_u64(value, u256_abs_i64(amount));
}

static inline u256 u256_shiftr_i64(const u256 value, const int64_t amount) {
  return u256_shiftr_u64(value, u256_abs_i64(amount));
}

static inline u256 u256_arith_shiftr_i64(const u256 value, const int64_t amount) {
  return u256_arith_shiftr_u64(value, u256_abs_i64(amount));
}

static inline u256 u256_of_fbits(const uint64_t value) {
  u256 result = {{0}};
  result.limbs[0] = value;
  return result;
}



static inline bytes32 bytes32_zero(void) {
  bytes32 result = {{0}};
  return result;
}

static inline bool eq_bytes32(const bytes32 op1, const bytes32 op2) {
  for (size_t i = 0; i < 4; ++i) {
    if (op1.lanes[i] != op2.lanes[i]) {
      return false;
    }
  }
  return true;
}


#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif

static inline u256 u256_from_bytes32(const bytes32 value) {
  u256 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    result.limbs[i] = value.lanes[i];
  }
  result.limbs[3] &= UINT64_MAX;
  return result;
}

static inline bytes32 bytes32_from_u256(const u256 value) {
  bytes32 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    result.lanes[i] = value.limbs[i];
  }
  result.lanes[3] &= UINT64_MAX;
  return result;
}

static inline bytes32 internal_vector_init_bytes32(const int64_t length_arg) {
  (void)length_arg;
  return bytes32_zero();
}

static inline bytes32 internal_vector_update_bytes32(
    bytes32 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 32) {
    const uint64_t i = (uint64_t)index;
    const uint64_t shift = (i & UINT64_C(7)) * UINT64_C(8);
    const uint64_t mask = UINT64_C(0xff) << shift;
    value.lanes[i >> 3] = (value.lanes[i >> 3] & ~mask) | (((uint64_t)(uint8_t)elem) << shift);
  }
  return value;
}

static inline uint64_t fast_vector_access_bytes32(const bytes32 value, const int64_t index) {
  if (index < 0 || index >= 32) {
    return UINT64_C(0);
  }
  const uint64_t i = (uint64_t)index;
  return (value.lanes[i >> 3] >> ((i & UINT64_C(7)) * UINT64_C(8))) & UINT64_C(0xff);
}

static inline bytes32 fast_unsigned_vector_update_bytes32(
    bytes32 value, const uint64_t index, const uint64_t elem) {
  if (index < 32) {
    const uint64_t shift = (index & UINT64_C(7)) * UINT64_C(8);
    const uint64_t mask = UINT64_C(0xff) << shift;
    value.lanes[index >> 3] =
        (value.lanes[index >> 3] & ~mask) | (((uint64_t)(uint8_t)elem) << shift);
  }
  return value;
}

static inline uint64_t fast_unsigned_vector_access_bytes32(
    const bytes32 value, const uint64_t index) {
  return index < 32
             ? ((value.lanes[index >> 3] >> ((index & UINT64_C(7)) * UINT64_C(8))) & UINT64_C(0xff))
             : UINT64_C(0);
}

static inline bytes32 fast_vector_init_bytes32(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  bytes32 result = {0};
  const uint64_t fill = UINT64_C(0x0101010101010101) * (uint8_t)elem;
  for (size_t i = 0; i < 4; ++i) {
    result.lanes[i] = fill;
  }
  result.lanes[3] &= UINT64_MAX;
  return result;
}

static inline bytes32 fast_unsigned_vector_init_bytes32(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  return fast_vector_init_bytes32((int64_t)length_arg, elem);
}



static inline bytes20 bytes20_zero(void) {
  bytes20 result = {{0}};
  return result;
}

static inline bool eq_bytes20(const bytes20 op1, const bytes20 op2) {
  for (size_t i = 0; i < 3; ++i) {
    if (op1.lanes[i] != op2.lanes[i]) {
      return false;
    }
  }
  return true;
}


#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif

static inline u256 u256_from_bytes20(const bytes20 value) {
  u256 result = {{0}};
  for (size_t i = 0; i < 3; ++i) {
    result.limbs[i] = value.lanes[i];
  }
  result.limbs[2] &= (UINT64_MAX >> 32);
  return result;
}

static inline bytes20 bytes20_from_u256(const u256 value) {
  bytes20 result = {{0}};
  for (size_t i = 0; i < 3; ++i) {
    result.lanes[i] = value.limbs[i];
  }
  result.lanes[2] &= (UINT64_MAX >> 32);
  return result;
}

static inline bytes20 internal_vector_init_bytes20(const int64_t length_arg) {
  (void)length_arg;
  return bytes20_zero();
}

static inline bytes20 internal_vector_update_bytes20(
    bytes20 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 20) {
    const uint64_t i = (uint64_t)index;
    const uint64_t shift = (i & UINT64_C(7)) * UINT64_C(8);
    const uint64_t mask = UINT64_C(0xff) << shift;
    value.lanes[i >> 3] = (value.lanes[i >> 3] & ~mask) | (((uint64_t)(uint8_t)elem) << shift);
  }
  return value;
}

static inline uint64_t fast_vector_access_bytes20(const bytes20 value, const int64_t index) {
  if (index < 0 || index >= 20) {
    return UINT64_C(0);
  }
  const uint64_t i = (uint64_t)index;
  return (value.lanes[i >> 3] >> ((i & UINT64_C(7)) * UINT64_C(8))) & UINT64_C(0xff);
}

static inline bytes20 fast_unsigned_vector_update_bytes20(
    bytes20 value, const uint64_t index, const uint64_t elem) {
  if (index < 20) {
    const uint64_t shift = (index & UINT64_C(7)) * UINT64_C(8);
    const uint64_t mask = UINT64_C(0xff) << shift;
    value.lanes[index >> 3] =
        (value.lanes[index >> 3] & ~mask) | (((uint64_t)(uint8_t)elem) << shift);
  }
  return value;
}

static inline uint64_t fast_unsigned_vector_access_bytes20(
    const bytes20 value, const uint64_t index) {
  return index < 20
             ? ((value.lanes[index >> 3] >> ((index & UINT64_C(7)) * UINT64_C(8))) & UINT64_C(0xff))
             : UINT64_C(0);
}

static inline bytes20 fast_vector_init_bytes20(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  bytes20 result = {0};
  const uint64_t fill = UINT64_C(0x0101010101010101) * (uint8_t)elem;
  for (size_t i = 0; i < 3; ++i) {
    result.lanes[i] = fill;
  }
  result.lanes[2] &= (UINT64_MAX >> 32);
  return result;
}

static inline bytes20 fast_unsigned_vector_init_bytes20(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  return fast_vector_init_bytes20((int64_t)length_arg, elem);
}


static inline bool EQUAL(AddressResult)(struct AddressResult op1, struct AddressResult op2) {
  return (bool)(EQUAL(bytes20)(op1.address, op2.address) && EQUAL(bool)(op1.success, op2.success));
}


static inline u320 u320_zero(void) {
  u320 result = {{0}};
  return result;
}

static inline u320 u320_of_u64(const uint64_t value) {
  u320 result = {{value, UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(0)}};
  return result;
}

static inline u320 u320_of_u128(const u128 value) {
  u320 result = {{
      value.limbs[0], value.limbs[1], UINT64_C(0), UINT64_C(0), UINT64_C(0)}};
  return result;
}

static inline u320 u320_of_u256(const u256 value) {
  u320 result = {{
      value.limbs[0], value.limbs[1], value.limbs[2], value.limbs[3],
      UINT64_C(0)}};
  return result;
}

static inline uint64_t u320_to_u64_unchecked(const u320 value) {
  return value.limbs[0];
}

static inline uint64_t u320_to_u64(const u320 value) {
  if ((value.limbs[1] | value.limbs[2] | value.limbs[3] | value.limbs[4])
      != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint64_t domain");
  }
  return u320_to_u64_unchecked(value);
}

static inline u128 u128_of_u320_unchecked(const u320 value) {
  u128 result = {{value.limbs[0], value.limbs[1]}};
  return result;
}

static inline u128 u128_of_u320(const u320 value) {
  if ((value.limbs[2] | value.limbs[3] | value.limbs[4]) != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint128_t domain");
  }
  return u128_of_u320_unchecked(value);
}

static inline u256 u256_of_u320_unchecked(const u320 value) {
  u256 result = {{
      value.limbs[0], value.limbs[1], value.limbs[2], value.limbs[3]}};
  return result;
}

static inline u256 u256_of_u320(const u320 value) {
  if (value.limbs[4] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint256_t domain");
  }
  return u256_of_u320_unchecked(value);
}

static inline bool eq_u320(const u320 lhs, const u320 rhs) {
  for (size_t i = 0; i < 5; ++i) {
    if (lhs.limbs[i] != rhs.limbs[i]) {
      return false;
    }
  }
  return true;
}

static inline bool u320_lt(const u320 lhs, const u320 rhs) {
  for (size_t i = 5; i-- > 0;) {
    if (lhs.limbs[i] != rhs.limbs[i]) {
      return lhs.limbs[i] < rhs.limbs[i];
    }
  }
  return false;
}

static inline u320 u320_add(const u320 lhs, const u320 rhs) {
  u320 result = {0};
  uint64_t carry = UINT64_C(0);
  for (size_t i = 0; i < 5; ++i) {
    const unsigned __int128 sum =
        (unsigned __int128)lhs.limbs[i] + rhs.limbs[i] + carry;
    result.limbs[i] = (uint64_t)sum;
    carry = (uint64_t)(sum >> 64);
  }
  return result;
}

static inline u320 u320_sub(const u320 lhs, const u320 rhs) {
  u320 result = {0};
  uint64_t borrow = UINT64_C(0);
  for (size_t i = 0; i < 5; ++i) {
    const uint64_t partial = lhs.limbs[i] - rhs.limbs[i];
    const uint64_t borrow1 = lhs.limbs[i] < rhs.limbs[i];
    result.limbs[i] = partial - borrow;
    const uint64_t borrow2 = partial < borrow;
    borrow = borrow1 | borrow2;
  }
  return result;
}

static inline u320 u320_mul(const u320 lhs, const u320 rhs) {
  u320 result = {{0}};
  for (size_t i = 0; i < 5; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; i + j < 5; ++j) {
      const size_t k = i + j;
      const unsigned __int128 sum =
          ((unsigned __int128)lhs.limbs[i] * rhs.limbs[j])
          + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
  }
  return result;
}

static inline u320 u320_identity(const u320 value) {
  return value;
}

static inline u320 u320_add_scalar(
    const u320 lhs, const uint64_t rhs) {
  u320 result = lhs;
  unsigned __int128 sum = (unsigned __int128)result.limbs[0] + rhs;
  result.limbs[0] = (uint64_t)sum;
  uint64_t carry = (uint64_t)(sum >> 64);
  for (size_t i = 1; i < 5 && carry != UINT64_C(0); ++i) {
    sum = (unsigned __int128)result.limbs[i] + carry;
    result.limbs[i] = (uint64_t)sum;
    carry = (uint64_t)(sum >> 64);
  }
  return result;
}

static inline u320 u320_mul_scalar(
    const u320 lhs, const uint64_t rhs) {
  u320 result = {0};
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < 5; ++i) {
    const unsigned __int128 product =
        ((unsigned __int128)lhs.limbs[i] * rhs) + carry;
    result.limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  return result;
}

/*
 * Select the widening conversion after specialization has fixed the final C
 * parameter type.  The specialization pass may narrow a function parameter
 * after arithmetic analysis, so baking the pre-specialization operand types
 * into a helper name would make the generated call stale.
 */
#define U320_SELECT_RHS(stem, rhs) \
  _Generic((rhs), \
      u320: stem##_u320, \
      u256: stem##_u256, \
      u128: stem##_u128, \
      default: stem##_u64)

#define U320_SELECT_BINARY(operation, lhs, rhs) \
  _Generic((lhs), \
      u320: U320_SELECT_RHS(u320_##operation##_u320, rhs), \
      u256: U320_SELECT_RHS(u320_##operation##_u256, rhs), \
      u128: U320_SELECT_RHS(u320_##operation##_u128, rhs), \
      default: U320_SELECT_RHS(u320_##operation##_u64, rhs))

#define u320_add_widen(lhs, rhs) \
  U320_SELECT_BINARY(add, lhs, rhs)((lhs), (rhs))

#define u320_mul_widen(lhs, rhs) \
  U320_SELECT_BINARY(mul, lhs, rhs)((lhs), (rhs))

static inline u320 u320_add_u320_u320(
    const u320 lhs, const u320 rhs) {
  return u320_add(lhs, rhs);
}

static inline u320 u320_add_u320_u256(
    const u320 lhs, const u256 rhs) {
  return u320_add(lhs, u320_of_u256(rhs));
}

static inline u320 u320_add_u320_u128(
    const u320 lhs, const u128 rhs) {
  return u320_add(lhs, u320_of_u128(rhs));
}

static inline u320 u320_add_u320_u64(
    const u320 lhs, const uint64_t rhs) {
  return u320_add_scalar(lhs, rhs);
}

static inline u320 u320_add_u256_u320(
    const u256 lhs, const u320 rhs) {
  return u320_add(rhs, u320_of_u256(lhs));
}

static inline u320 u320_add_u256_u256(
    const u256 lhs, const u256 rhs) {
  return u320_add(u320_of_u256(lhs), u320_of_u256(rhs));
}

static inline u320 u320_add_u256_u128(
    const u256 lhs, const u128 rhs) {
  return u320_add(u320_of_u256(lhs), u320_of_u128(rhs));
}

static inline u320 u320_add_u128_u256(
    const u128 lhs, const u256 rhs) {
  return u320_add_u256_u128(rhs, lhs);
}

static inline u320 u320_add_u256_u64(
    const u256 lhs, const uint64_t rhs) {
  return u320_add_scalar(u320_of_u256(lhs), rhs);
}

static inline u320 u320_add_u64_u256(
    const uint64_t lhs, const u256 rhs) {
  return u320_add_u256_u64(rhs, lhs);
}

static inline u320 u320_add_u128_u128(
    const u128 lhs, const u128 rhs) {
  return u320_add(u320_of_u128(lhs), u320_of_u128(rhs));
}

static inline u320 u320_add_u128_u320(
    const u128 lhs, const u320 rhs) {
  return u320_add(rhs, u320_of_u128(lhs));
}

static inline u320 u320_add_u128_u64(
    const u128 lhs, const uint64_t rhs) {
  return u320_add_scalar(u320_of_u128(lhs), rhs);
}

static inline u320 u320_add_u64_u128(
    const uint64_t lhs, const u128 rhs) {
  return u320_add_u128_u64(rhs, lhs);
}

static inline u320 u320_add_u64_u64(
    const uint64_t lhs, const uint64_t rhs) {
  return u320_add_scalar(u320_of_u64(lhs), rhs);
}

static inline u320 u320_add_u64_u320(
    const uint64_t lhs, const u320 rhs) {
  return u320_add_scalar(rhs, lhs);
}

static inline u320 u320_mul_u320_u320(
    const u320 lhs, const u320 rhs) {
  return u320_mul(lhs, rhs);
}

static inline u320 u320_mul_u320_u256(
    const u320 lhs, const u256 rhs) {
  return u320_mul(lhs, u320_of_u256(rhs));
}

static inline u320 u320_mul_u320_u128(
    const u320 lhs, const u128 rhs) {
  return u320_mul(lhs, u320_of_u128(rhs));
}

static inline u320 u320_mul_u320_u64(
    const u320 lhs, const uint64_t rhs) {
  return u320_mul_scalar(lhs, rhs);
}

static inline u320 u320_mul_u256_u320(
    const u256 lhs, const u320 rhs) {
  return u320_mul(rhs, u320_of_u256(lhs));
}

static inline u320 u320_mul_u256_u256(
    const u256 lhs, const u256 rhs) {
  return u320_mul(u320_of_u256(lhs), u320_of_u256(rhs));
}

static inline u320 u320_mul_u256_u128(
    const u256 lhs, const u128 rhs) {
  return u320_mul(u320_of_u256(lhs), u320_of_u128(rhs));
}

static inline u320 u320_mul_u128_u256(
    const u128 lhs, const u256 rhs) {
  return u320_mul_u256_u128(rhs, lhs);
}

static inline u320 u320_mul_u256_u64(
    const u256 lhs, const uint64_t rhs) {
  return u320_mul_scalar(u320_of_u256(lhs), rhs);
}

static inline u320 u320_mul_u64_u256(
    const uint64_t lhs, const u256 rhs) {
  return u320_mul_u256_u64(rhs, lhs);
}

static inline u320 u320_mul_u128_u128(
    const u128 lhs, const u128 rhs) {
  return u320_mul(u320_of_u128(lhs), u320_of_u128(rhs));
}

static inline u320 u320_mul_u128_u320(
    const u128 lhs, const u320 rhs) {
  return u320_mul(rhs, u320_of_u128(lhs));
}

static inline u320 u320_mul_u128_u64(
    const u128 lhs, const uint64_t rhs) {
  return u320_mul_scalar(u320_of_u128(lhs), rhs);
}

static inline u320 u320_mul_u64_u128(
    const uint64_t lhs, const u128 rhs) {
  return u320_mul_u128_u64(rhs, lhs);
}

static inline u320 u320_mul_u64_u64(
    const uint64_t lhs, const uint64_t rhs) {
  return u320_mul_scalar(u320_of_u64(lhs), rhs);
}

static inline u320 u320_mul_u64_u320(
    const uint64_t lhs, const u320 rhs) {
  return u320_mul_scalar(rhs, lhs);
}

static inline u256 u256_sub_u320(
    const u256 lhs, const u320 rhs) {
  return u256_of_u320(u320_sub(u320_of_u256(lhs), rhs));
}

static inline u128 u128_sub_u320(
    const u128 lhs, const u320 rhs) {
  return u128_of_u320(u320_sub(u320_of_u128(lhs), rhs));
}

static inline uint64_t u64_sub_u320(
    const uint64_t lhs, const u320 rhs) {
  return u320_to_u64(u320_sub(u320_of_u64(lhs), rhs));
}

/* Base-2^64 long division for the common five-by-one-limb shape. */
static inline void u320_divrem_u64(
    const u320 dividend,
    const uint64_t divisor,
    u320 *quotient,
    uint64_t *remainder) {
  u320 q = {{0}};
  uint64_t rem = UINT64_C(0);
  for (size_t i = 5; i-- > 0;) {
    const unsigned __int128 partial =
        ((unsigned __int128)rem << 64) | dividend.limbs[i];
    q.limbs[i] = (uint64_t)(partial / divisor);
    rem = (uint64_t)(partial % divisor);
  }
  if (quotient != NULL) {
    *quotient = q;
  }
  if (remainder != NULL) {
    *remainder = rem;
  }
}

static inline u320 u320_div_u64(
    const u320 dividend, const uint64_t divisor) {
  u320 result;
  u320_divrem_u64(dividend, divisor, &result, NULL);
  return result;
}

static inline uint64_t u320_mod_u64(
    const u320 dividend, const uint64_t divisor) {
  uint64_t result;
  u320_divrem_u64(dividend, divisor, NULL, &result);
  return result;
}

static inline bool u320_is_zero(const u320 value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2]
          | value.limbs[3] | value.limbs[4]) == UINT64_C(0);
}

static inline bool u320_power_of_two_shift(
    const u320 value, uint32_t *shift) {
  uint32_t found = UINT32_MAX;
  for (uint32_t i = 0; i < 5; ++i) {
    const uint64_t limb = value.limbs[i];
    if (limb == UINT64_C(0)) {
      continue;
    }
    if ((limb & (limb - UINT64_C(1))) != UINT64_C(0) || found != UINT32_MAX) {
      return false;
    }
    found = (i * 64U) + (uint32_t)__builtin_ctzll(limb);
  }
  if (found == UINT32_MAX) {
    return false;
  }
  *shift = found;
  return true;
}

static inline u320 u320_shr(const u320 value, const uint32_t shift) {
  u320 result = {{0}};
  if (shift >= 320U) {
    return result;
  }
  const uint32_t words = shift / 64U;
  const uint32_t bits = shift % 64U;
  for (uint32_t out = 0; out + words < 5U; ++out) {
    const uint32_t in = out + words;
    result.limbs[out] = value.limbs[in] >> bits;
    if (bits != 0U && in + 1U < 5U) {
      result.limbs[out] |= value.limbs[in + 1U] << (64U - bits);
    }
  }
  return result;
}

static inline u320 u320_mod_power_of_two(
    const u320 value, const uint32_t shift) {
  if (shift >= 320U) {
    return value;
  }
  u320 result = value;
  const uint32_t word = shift / 64U;
  const uint32_t bits = shift % 64U;
  if (bits == 0U) {
    for (uint32_t i = word; i < 5U; ++i) {
      result.limbs[i] = UINT64_C(0);
    }
  } else {
    result.limbs[word] &= (UINT64_C(1) << bits) - UINT64_C(1);
    for (uint32_t i = word + 1U; i < 5U; ++i) {
      result.limbs[i] = UINT64_C(0);
    }
  }
  return result;
}

/* The general fallback is a fixed-width restoring division.  The generated
 * EVM recurrence takes the one-limb fast path at runtime; the fallback keeps
 * the representation complete for other proved U320 expressions. */
static inline void u320_divrem(
    const u320 dividend,
    const u320 divisor,
    u320 *quotient,
    u320 *remainder) {
  if (u320_is_zero(divisor)) {
    sail_native_conversion_failure("division by zero");
  }
  if ((divisor.limbs[1] | divisor.limbs[2]
       | divisor.limbs[3] | divisor.limbs[4]) == UINT64_C(0)) {
    u320 q;
    uint64_t r;
    u320_divrem_u64(dividend, divisor.limbs[0], &q, &r);
    if (quotient != NULL) {
      *quotient = q;
    }
    if (remainder != NULL) {
      *remainder = u320_of_u64(r);
    }
    return;
  }
  uint32_t shift;
  if (u320_power_of_two_shift(divisor, &shift)) {
    if (quotient != NULL) {
      *quotient = u320_shr(dividend, shift);
    }
    if (remainder != NULL) {
      *remainder = u320_mod_power_of_two(dividend, shift);
    }
    return;
  }

  u320 q = {{0}};
  u320 rem = {{0}};
  for (uint32_t bit = 320U; bit-- > 0U;) {
    const uint64_t carry = rem.limbs[4] >> 63;
    for (uint32_t i = 4U; i > 0U; --i) {
      rem.limbs[i] = (rem.limbs[i] << 1) | (rem.limbs[i - 1U] >> 63);
    }
    rem.limbs[0] =
        (rem.limbs[0] << 1)
        | ((dividend.limbs[bit / 64U] >> (bit % 64U)) & UINT64_C(1));
    if (carry != UINT64_C(0) || !u320_lt(rem, divisor)) {
      rem = u320_sub(rem, divisor);
      q.limbs[bit / 64U] |= UINT64_C(1) << (bit % 64U);
    }
  }
  if (quotient != NULL) {
    *quotient = q;
  }
  if (remainder != NULL) {
    *remainder = rem;
  }
}

static inline u320 u320_div(
    const u320 dividend, const u320 divisor) {
  u320 result;
  u320_divrem(dividend, divisor, &result, NULL);
  return result;
}

static inline u320 u320_mod(
    const u320 dividend, const u320 divisor) {
  u320 result;
  u320_divrem(dividend, divisor, NULL, &result);
  return result;
}




static inline u128 u128_zero(void) {
  u128 result = {{0}};
  return result;
}

static inline u128 u128_of_u64(const uint64_t value) {
  u128 result = {{value, UINT64_C(0)}};
  return result;
}

static inline uint64_t u128_to_u64_unchecked(const u128 value) {
  return value.limbs[0];
}

static inline uint64_t u128_to_u64(const u128 value) {
  if (value.limbs[1] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint64_t domain");
  }
  return u128_to_u64_unchecked(value);
}

static inline uint64_t u128_extract_u64(const u128 value, const uint64_t start) {
  if (start >= UINT64_C(128)) {
    return UINT64_C(0);
  }
  const size_t limb = (size_t)(start >> 6);
  const unsigned offset = (unsigned)(start & UINT64_C(63));
  uint64_t result = value.limbs[limb] >> offset;
  if (offset != 0 && limb + 1 < 2) {
    result |= value.limbs[limb + 1] << (64 - offset);
  }
  return result;
}

static inline bool eq_u128(const u128 op1, const u128 op2) {
  return (bool)(op1.limbs[0] == op2.limbs[0] && op1.limbs[1] == op2.limbs[1]);
}

static inline bool u128_eq_u64(const u128 lhs, const uint64_t rhs) {
  return (bool)(lhs.limbs[0] == rhs && lhs.limbs[1] == UINT64_C(0));
}

static inline u128 u128_not(const u128 value) {
  u128 result = {{~value.limbs[0], ~value.limbs[1]}};
  return result;
}

static inline u128 u128_and(const u128 lhs, const u128 rhs) {
  u128 result = {{lhs.limbs[0] & rhs.limbs[0], lhs.limbs[1] & rhs.limbs[1]}};
  return result;
}

static inline u128 u128_or(const u128 lhs, const u128 rhs) {
  u128 result = {{lhs.limbs[0] | rhs.limbs[0], lhs.limbs[1] | rhs.limbs[1]}};
  return result;
}

static inline u128 u128_xor(const u128 lhs, const u128 rhs) {
  u128 result = {{lhs.limbs[0] ^ rhs.limbs[0], lhs.limbs[1] ^ rhs.limbs[1]}};
  return result;
}

static inline bool u128_lt(const u128 lhs, const u128 rhs) {
  return (bool)(lhs.limbs[1] != rhs.limbs[1]
      ? lhs.limbs[1] < rhs.limbs[1]
      : lhs.limbs[0] < rhs.limbs[0]);
}

static inline bool u128_lt_u64(const u128 lhs, const uint64_t rhs) {
  return (bool)(lhs.limbs[1] == UINT64_C(0) && lhs.limbs[0] < rhs);
}

static inline bool u64_lt_u128(const uint64_t lhs, const u128 rhs) {
  return (bool)(rhs.limbs[1] != UINT64_C(0) || lhs < rhs.limbs[0]);
}

static inline u128 u128_add(const u128 lhs, const u128 rhs) {
  u128 result;
  result.limbs[0] = lhs.limbs[0] + rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] + rhs.limbs[1]
                  + (result.limbs[0] < lhs.limbs[0]);
  return result;
}

static inline u128 u128_add_u64(const u128 lhs, const uint64_t rhs) {
  u128 result = lhs;
  result.limbs[0] += rhs;
  result.limbs[1] += result.limbs[0] < lhs.limbs[0];
  return result;
}

static inline u128 u128_add_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  u128 result = {{lhs + rhs, lhs + rhs < lhs}};
  return result;
}

static inline u128 u128_sub(const u128 lhs, const u128 rhs) {
  u128 result;
  result.limbs[0] = lhs.limbs[0] - rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] - rhs.limbs[1]
                  - (lhs.limbs[0] < rhs.limbs[0]);
  return result;
}

static inline u128 u128_sub_u64(const u128 lhs, const uint64_t rhs) {
  u128 result = lhs;
  result.limbs[0] -= rhs;
  result.limbs[1] -= lhs.limbs[0] < rhs;
  return result;
}

static inline uint64_t u64_sub_u128(const uint64_t lhs, const u128 rhs) {
  return lhs - rhs.limbs[0];
}

static inline u128 u128_sub_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  u128 result = {{lhs - rhs, UINT64_C(0)}};
  return result;
}

static inline u128 u128_mul(const u128 lhs, const u128 rhs) {
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] * rhs.limbs[0];
  u128 result;
  result.limbs[0] = (uint64_t)low;
  result.limbs[1] = (uint64_t)(low >> 64)
                  + (lhs.limbs[0] * rhs.limbs[1])
                  + (lhs.limbs[1] * rhs.limbs[0]);
  return result;
}

static inline u128 u128_mul_u64(const u128 lhs, const uint64_t rhs) {
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] * rhs;
  u128 result;
  result.limbs[0] = (uint64_t)low;
  result.limbs[1] = (uint64_t)(low >> 64) + (lhs.limbs[1] * rhs);
  return result;
}

static inline u128 u128_mul_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  const unsigned __int128 product = (unsigned __int128)lhs * rhs;
  u128 result = {{(uint64_t)product, (uint64_t)(product >> 64)}};
  return result;
}

static inline bool u128_is_zero(const u128 value) {
  return (value.limbs[0] | value.limbs[1]) == UINT64_C(0);
}

/* Integer division helpers are emitted only for JIB operations carrying the
 * nonzero-divisor proof marker.  Do not re-check that source invariant here:
 * an unproved division remains in Sail's mathematical integer runtime. */
static inline void u128_divrem_u64(const u128 dividend,
                                   const uint64_t divisor,
  u128 *quotient,
                                   uint64_t *remainder) {
  u128 q = {{0}};
  uint64_t r = UINT64_C(0);
  q.limbs[1] = dividend.limbs[1] / divisor;
  r = dividend.limbs[1] % divisor;
  const unsigned __int128 partial =
      ((unsigned __int128)r << 64) | dividend.limbs[0];
  q.limbs[0] = (uint64_t)(partial / divisor);
  r = (uint64_t)(partial % divisor);
  if (quotient != NULL) {
    *quotient = q;
  }
  if (remainder != NULL) {
    *remainder = r;
  }
}

static inline void u128_store_divrem(const u128 quotient_value,
                                     const u128 remainder_value,
                                     u128 *quotient,
                                     u128 *remainder) {
  if (quotient != NULL) {
    *quotient = quotient_value;
  }
  if (remainder != NULL) {
    *remainder = remainder_value;
  }
}

/* Two-limb specialization of normalized Knuth division.  Like ruint, it
 * dispatches the one-limb divisor case separately and computes at most one
 * quotient limb for a normalized two-limb divisor. */
static inline void u128_divrem(const u128 dividend,
                               const u128 divisor,
                               u128 *quotient,
  u128 *remainder) {
  u128 q = {{0}};
  u128 r = {{0}};
  if (divisor.limbs[1] == UINT64_C(0)) {
    uint64_t rem = UINT64_C(0);
    u128_divrem_u64(dividend, divisor.limbs[0], &q, &rem);
    r.limbs[0] = rem;
    u128_store_divrem(q, r, quotient, remainder);
    return;
  }

  if (u128_lt(dividend, divisor)) {
    r = dividend;
    u128_store_divrem(q, r, quotient, remainder);
    return;
  }

  const unsigned shift = (unsigned)__builtin_clzll(divisor.limbs[1]);
  uint64_t v0;
  uint64_t v1;
  uint64_t u0;
  uint64_t u1;
  uint64_t u2;
  if (shift == 0) {
    v0 = divisor.limbs[0];
    v1 = divisor.limbs[1];
    u0 = dividend.limbs[0];
    u1 = dividend.limbs[1];
    u2 = UINT64_C(0);
  } else {
    v0 = divisor.limbs[0] << shift;
    v1 = (divisor.limbs[1] << shift)
       | (divisor.limbs[0] >> (64 - shift));
    u0 = dividend.limbs[0] << shift;
    u1 = (dividend.limbs[1] << shift)
       | (dividend.limbs[0] >> (64 - shift));
    u2 = dividend.limbs[1] >> (64 - shift);
  }

  uint64_t qhat;
  uint64_t rhat;
  bool rhat_overflow = false;
  if (u2 == v1) {
    qhat = UINT64_MAX;
    rhat = u1 + v1;
    rhat_overflow = rhat < u1;
  } else {
    const unsigned __int128 top = ((unsigned __int128)u2 << 64) | u1;
    qhat = (uint64_t)(top / v1);
    rhat = (uint64_t)(top % v1);
  }
  while (!rhat_overflow
         && (unsigned __int128)qhat * v0
                > (((unsigned __int128)rhat << 64) | u0)) {
    qhat--;
    const uint64_t next = rhat + v1;
    rhat_overflow = next < rhat;
    rhat = next;
  }

  const unsigned __int128 p0 = (unsigned __int128)qhat * v0;
  const uint64_t p0_low = (uint64_t)p0;
  const uint64_t p0_high = (uint64_t)(p0 >> 64);
  const uint64_t borrow0 = u0 < p0_low;
  u0 -= p0_low;

  const unsigned __int128 p1 = ((unsigned __int128)qhat * v1)
                              + p0_high + borrow0;
  const uint64_t p1_low = (uint64_t)p1;
  const uint64_t p1_high = (uint64_t)(p1 >> 64);
  const uint64_t borrow1 = u1 < p1_low;
  u1 -= p1_low;
  const uint64_t top_subtrahend = p1_high + borrow1;
  const bool top_overflow = top_subtrahend < p1_high;
  const bool negative = (bool)(top_overflow || u2 < top_subtrahend);

  if (negative) {
    qhat--;
    const uint64_t previous = u0;
    u0 += v0;
    const uint64_t carry = u0 < previous;
    u1 += v1 + carry;
  }
  q.limbs[0] = qhat;
  if (shift == 0) {
    r.limbs[0] = u0;
    r.limbs[1] = u1;
  } else {
    r.limbs[0] = (u0 >> shift) | (u1 << (64 - shift));
    r.limbs[1] = u1 >> shift;
  }

  u128_store_divrem(q, r, quotient, remainder);
}

static inline u128 u128_div(const u128 lhs, const u128 rhs) {
  u128 result;
  u128_divrem(lhs, rhs, &result, NULL);
  return result;
}

static inline u128 u128_mod(const u128 lhs, const u128 rhs) {
  u128 result;
  u128_divrem(lhs, rhs, NULL, &result);
  return result;
}

static inline u128 u128_div_u64(const u128 lhs, const uint64_t rhs) {
  u128 result;
  u128_divrem_u64(lhs, rhs, &result, NULL);
  return result;
}

static inline u128 u128_mod_u64(const u128 lhs, const uint64_t rhs) {
  uint64_t remainder;
  u128_divrem_u64(lhs, rhs, NULL, &remainder);
  return u128_of_u64(remainder);
}


static inline bool EQUAL(PrecompileId)(enum PrecompileId op1, enum PrecompileId op2) {
  return (bool)(op1 == op2);
}

static inline enum PrecompileId UNDEFINED(PrecompileId)(void) { return NotPrecompile; }

static inline bool EQUAL(MemoryRangeFields)(struct MemoryRangeFields op1, struct MemoryRangeFields op2) {
  return (bool)((op1.len == op2.len) && (op1.off == op2.off));
}

static inline bool EQUAL(MemoryAccessFields)(struct MemoryAccessFields op1, struct MemoryAccessFields op2) {
  return (bool)(EQUAL(MemoryRangeFields)(op1.range, op2.range) && (op1.required_size == op2.required_size));
}

static inline bool EQUAL(GasCharge)(struct GasCharge op1, struct GasCharge op2) {
  return (bool)(EQUAL(bool)(op1.affordable, op2.affordable) && (op1.cost == op2.cost));
}

static inline bool EQUAL(LogData)(struct LogData op1, struct LogData op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_LogDataMemory: {
    return EQUAL(Bytes)(op1.variants.LogDataMemory, op2.variants.LogDataMemory);
  }
  case Kind_LogDataWord: {
    return EQUAL(u256)(op1.variants.LogDataWord, op2.variants.LogDataWord);
  }
  }
  return false;
}

static inline struct LogData LogDataMemory(Bytes op) {
  struct LogData result;
  result.kind = Kind_LogDataMemory;
  result.variants.LogDataMemory = op;
  return result;
}

static inline struct LogData LogDataWord(u256 op) {
  struct LogData result;
  result.kind = Kind_LogDataWord;
  result.variants.LogDataWord = op;
  return result;
}

static inline bool EQUAL(CalldataSlice)(struct CalldataSlice op1, struct CalldataSlice op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_InputCalldata: {
    return EQUAL(Bytes)(op1.variants.InputCalldata, op2.variants.InputCalldata);
  }
  case Kind_MemoryCalldata: {
    return EQUAL(Bytes)(op1.variants.MemoryCalldata, op2.variants.MemoryCalldata);
  }
  }
  return false;
}

static inline struct CalldataSlice InputCalldata(Bytes op) {
  struct CalldataSlice result;
  result.kind = Kind_InputCalldata;
  result.variants.InputCalldata = op;
  return result;
}

static inline struct CalldataSlice MemoryCalldata(Bytes op) {
  struct CalldataSlice result;
  result.kind = Kind_MemoryCalldata;
  result.variants.MemoryCalldata = op;
  return result;
}

static inline bool EQUAL(FatalError)(enum FatalError op1, enum FatalError op2) {
  return (bool)(op1 == op2);
}

static inline enum FatalError UNDEFINED(FatalError)(void) { return InvalidConfig; }

static inline bool EQUAL(ExceptionKind)(enum ExceptionKind op1, enum ExceptionKind op2) {
  return (bool)(op1 == op2);
}

static inline enum ExceptionKind UNDEFINED(ExceptionKind)(void) { return StackUnderflow; }

static inline bool EQUAL(tuple_bool_uint_32_uint_64)(struct tuple_bool_uint_32_uint_64 op1, struct tuple_bool_uint_32_uint_64 op2) {
  return (bool)(EQUAL(bool)(op1.tup0, op2.tup0) && (op1.tup1 == op2.tup1) && (op1.tup2 == op2.tup2));
}

static inline bool EQUAL(HaltKind)(struct HaltKind op1, struct HaltKind op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_HaltReturn: {
    return EQUAL(Bytes)(op1.variants.HaltReturn, op2.variants.HaltReturn);
  }
  case Kind_HaltRevert: {
    return EQUAL(Bytes)(op1.variants.HaltRevert, op2.variants.HaltRevert);
  }
  case Kind_HaltSelfDestruct:
  case Kind_HaltStop: {
    return true;
  }
  }
  return false;
}

static inline struct HaltKind HaltReturn(Bytes op) {
  struct HaltKind result;
  result.kind = Kind_HaltReturn;
  result.variants.HaltReturn = op;
  return result;
}

static inline struct HaltKind HaltRevert(Bytes op) {
  struct HaltKind result;
  result.kind = Kind_HaltRevert;
  result.variants.HaltRevert = op;
  return result;
}

static inline struct HaltKind HaltSelfDestruct(unit op) {
  struct HaltKind result;
  result.kind = Kind_HaltSelfDestruct;
  result.variants.HaltSelfDestruct = op;
  return result;
}

static inline struct HaltKind HaltStop(unit op) {
  struct HaltKind result;
  result.kind = Kind_HaltStop;
  result.variants.HaltStop = op;
  return result;
}

static inline bool EQUAL(FrameStatus)(struct FrameStatus op1, struct FrameStatus op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_Exceptional: {
    return EQUAL(ExceptionKind)(op1.variants.Exceptional, op2.variants.Exceptional);
  }
  case Kind_Halted: {
    return EQUAL(HaltKind)(op1.variants.Halted, op2.variants.Halted);
  }
  case Kind_Running: {
    return true;
  }
  }
  return false;
}

static inline struct FrameStatus Exceptional(enum ExceptionKind op) {
  struct FrameStatus result;
  result.kind = Kind_Exceptional;
  result.variants.Exceptional = op;
  return result;
}

static inline struct FrameStatus Halted(struct HaltKind op) {
  struct FrameStatus result;
  result.kind = Kind_Halted;
  result.variants.Halted = op;
  return result;
}

static inline struct FrameStatus Running(unit op) {
  struct FrameStatus result;
  result.kind = Kind_Running;
  result.variants.Running = op;
  return result;
}

static inline bool EQUAL(BoundedSszListRef)(struct BoundedSszListRef op1, struct BoundedSszListRef op2) {
  return (bool)(EQUAL(Bytes)(op1.bytes, op2.bytes) && (op1.count == op2.count) && (op1.max_item_length == op2.max_item_length));
}

static inline bool EQUAL(BoundedSszListCursor)(struct BoundedSszListCursor op1, struct BoundedSszListCursor op2) {
  return (bool)((op1.current == op2.current) && (op1.index == op2.index) && EQUAL(BoundedSszListRef)(op1.items, op2.items));
}

static inline bool EQUAL(tuple_Bytes_BoundedSszListRef)(struct tuple_Bytes_BoundedSszListRef op1, struct tuple_Bytes_BoundedSszListRef op2) {
  return (bool)(EQUAL(Bytes)(op1.tup0, op2.tup0) && EQUAL(BoundedSszListRef)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_Bytes_BoundedSszListCursor)(struct tuple_Bytes_BoundedSszListCursor op1, struct tuple_Bytes_BoundedSszListCursor op2) {
  return (bool)(EQUAL(Bytes)(op1.tup0, op2.tup0) && EQUAL(BoundedSszListCursor)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(DeepStackOperation)(enum DeepStackOperation op1, enum DeepStackOperation op2) {
  return (bool)(op1 == op2);
}

static inline enum DeepStackOperation UNDEFINED(DeepStackOperation)(void) { return DeepStackDuplicate; }

static inline bool EQUAL(CodeFields)(struct CodeFields op1, struct CodeFields op2) {
  return (bool)((op1.bytes == op2.bytes) && (op1.jumpdests == op2.jumpdests) && (op1.len == op2.len));
}

static inline bool EQUAL(ScratchRlpFieldRef)(struct ScratchRlpFieldRef op1, struct ScratchRlpFieldRef op2) {
  return (bool)((op1.content_len == op2.content_len) && EQUAL(bool)(op1.is_list, op2.is_list) && EQUAL(Bytes)(op1.source, op2.source));
}

static inline bool EQUAL(RlpFieldRef)(struct RlpFieldRef op1, struct RlpFieldRef op2) {
  return (bool)((op1.content_len == op2.content_len) && EQUAL(bool)(op1.is_list, op2.is_list) && EQUAL(Bytes)(op1.source, op2.source));
}

static inline bool EQUAL(tuple_bool_bytes20)(struct tuple_bool_bytes20 op1, struct tuple_bool_bytes20 op2) {
  return (bool)(EQUAL(bool)(op1.tup0, op2.tup0) && EQUAL(bytes20)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(GasLimitsFields)(struct GasLimitsFields op1, struct GasLimitsFields op2) {
  return (bool)((op1.block_limit == op2.block_limit) && (op1.system_regular_limit == op2.system_regular_limit) && (op1.system_state_limit == op2.system_state_limit) && (op1.transaction_regular_limit == op2.transaction_regular_limit) && (op1.transaction_total_limit == op2.transaction_total_limit));
}

static inline bool EQUAL(ChainConfig)(struct ChainConfig op1, struct ChainConfig op2) {
  return (bool)(op1.chain_id == op2.chain_id);
}

static inline bool EQUAL(BlobScheduleFields)(struct BlobScheduleFields op1, struct BlobScheduleFields op2) {
  return (bool)((op1.base_fee_update_fraction == op2.base_fee_update_fraction) && (op1.max == op2.max) && (op1.target == op2.target));
}

static inline bool EQUAL(ProtocolProfileFields)(struct ProtocolProfileFields op1, struct ProtocolProfileFields op2) {
  return (bool)(EQUAL(BlobScheduleFields)(op1.blob_schedule, op2.blob_schedule) && (op1.deployed_code_size_limit == op2.deployed_code_size_limit) && (op1.excess_blob_gas_limit == op2.excess_blob_gas_limit) && (op1.fork == op2.fork) && (op1.initcode_size_limit == op2.initcode_size_limit) && (op1.refund_divisor == op2.refund_divisor) && (op1.transaction_blob_limit == op2.transaction_blob_limit) && (op1.transaction_regular_gas_limit == op2.transaction_regular_gas_limit) && (op1.transaction_total_gas_limit == op2.transaction_total_gas_limit));
}

static inline bool EQUAL(ExecutionProfileFields)(struct ExecutionProfileFields op1, struct ExecutionProfileFields op2) {
  return (bool)(EQUAL(GasLimitsFields)(op1.gas, op2.gas) && EQUAL(ProtocolProfileFields)(op1.protocol, op2.protocol));
}

static inline bool EQUAL(StorageValue)(struct StorageValue op1, struct StorageValue op2) {
  return (bool)(EQUAL(u256)(op1.curr, op2.curr) && EQUAL(u256)(op1.orig, op2.orig));
}

static inline bool EQUAL(StorageKey)(struct StorageKey op1, struct StorageKey op2) {
  return (bool)(EQUAL(bytes20)(op1.addr, op2.addr) && EQUAL(u256)(op1.slot, op2.slot));
}

static inline bool EQUAL(StorageEntry)(struct StorageEntry op1, struct StorageEntry op2) {
  return (bool)(EQUAL(StorageKey)(op1.key, op2.key) && EQUAL(StorageValue)(op1.value, op2.value));
}

static inline bool EQUAL(StorageTrieEntry)(struct StorageTrieEntry op1, struct StorageTrieEntry op2) {
  return (bool)(EQUAL(bytes32)(op1.address_hash, op2.address_hash) && EQUAL(StorageEntry)(op1.entry, op2.entry) && EQUAL(bytes32)(op1.slot_hash, op2.slot_hash));
}

static inline bool EQUAL(StorageBlockIterResult)(struct StorageBlockIterResult op1, struct StorageBlockIterResult op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_StorageBlockIterRow: {
    return EQUAL(StorageTrieEntry)(op1.variants.StorageBlockIterRow, op2.variants.StorageBlockIterRow);
  }
  case Kind_StorageBlockIterExhausted: {
    return true;
  }
  }
  return false;
}

static inline struct StorageBlockIterResult StorageBlockIterExhausted(unit op) {
  struct StorageBlockIterResult result;
  result.kind = Kind_StorageBlockIterExhausted;
  result.variants.StorageBlockIterExhausted = op;
  return result;
}

static inline struct StorageBlockIterResult StorageBlockIterRow(struct StorageTrieEntry op) {
  struct StorageBlockIterResult result;
  result.kind = Kind_StorageBlockIterRow;
  result.variants.StorageBlockIterRow = op;
  return result;
}

static inline bool EQUAL(AccountInfo)(struct AccountInfo op1, struct AccountInfo op2) {
  return (bool)(EQUAL(u256)(op1.balance, op2.balance) && EQUAL(bytes32)(op1.code_hash, op2.code_hash) && (op1.nonce == op2.nonce) && EQUAL(bytes32)(op1.storage_root, op2.storage_root));
}

static inline bool EQUAL(Account)(struct Account op1, struct Account op2) {
  return (bool)(EQUAL(bool)(op1.created, op2.created) && EQUAL(AccountInfo)(op1.info, op2.info) && EQUAL(bool)(op1.present, op2.present) && EQUAL(bool)(op1.selfdestructed, op2.selfdestructed) && EQUAL(bool)(op1.storage_cleared, op2.storage_cleared));
}

static inline bool EQUAL(AcctValue)(struct AcctValue op1, struct AcctValue op2) {
  return (bool)(EQUAL(Account)(op1.curr, op2.curr) && EQUAL(Account)(op1.orig, op2.orig));
}

static inline bool EQUAL(AcctEntry)(struct AcctEntry op1, struct AcctEntry op2) {
  return (bool)(EQUAL(bytes20)(op1.addr, op2.addr) && EQUAL(AcctValue)(op1.value, op2.value));
}

static inline bool EQUAL(AcctTrieEntry)(struct AcctTrieEntry op1, struct AcctTrieEntry op2) {
  return (bool)(EQUAL(bytes32)(op1.address_hash, op2.address_hash) && EQUAL(AcctEntry)(op1.entry, op2.entry));
}

static inline bool EQUAL(AcctBlockIterResult)(struct AcctBlockIterResult op1, struct AcctBlockIterResult op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_AcctBlockIterRow: {
    return EQUAL(AcctTrieEntry)(op1.variants.AcctBlockIterRow, op2.variants.AcctBlockIterRow);
  }
  case Kind_AcctBlockIterExhausted: {
    return true;
  }
  }
  return false;
}

static inline struct AcctBlockIterResult AcctBlockIterExhausted(unit op) {
  struct AcctBlockIterResult result;
  result.kind = Kind_AcctBlockIterExhausted;
  result.variants.AcctBlockIterExhausted = op;
  return result;
}

static inline struct AcctBlockIterResult AcctBlockIterRow(struct AcctTrieEntry op) {
  struct AcctBlockIterResult result;
  result.kind = Kind_AcctBlockIterRow;
  result.variants.AcctBlockIterRow = op;
  return result;
}

static inline bool EQUAL(TxType)(enum TxType op1, enum TxType op2) {
  return (bool)(op1 == op2);
}

static inline enum TxType UNDEFINED(TxType)(void) { return LegacyTx; }

static inline bool EQUAL(TxSignatureScheme)(enum TxSignatureScheme op1, enum TxSignatureScheme op2) {
  return (bool)(op1 == op2);
}

static inline enum TxSignatureScheme UNDEFINED(TxSignatureScheme)(void) { return LegacySignature; }

static inline bool EQUAL(TxTypeSemantics)(struct TxTypeSemantics op1, struct TxTypeSemantics op2) {
  return (bool)(EQUAL(bool)(op1.blob, op2.blob) && (op1.minimum_fork == op2.minimum_fork) && EQUAL(bool)(op1.set_code, op2.set_code) && EQUAL(TxSignatureScheme)(op1.signature, op2.signature));
}

static inline bool EQUAL(tuple_u256_u256)(struct tuple_u256_u256 op1, struct tuple_u256_u256 op2) {
  return (bool)(EQUAL(u256)(op1.tup0, op2.tup0) && EQUAL(u256)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_u256_u256_u256)(struct tuple_u256_u256_u256 op1, struct tuple_u256_u256_u256 op2) {
  return (bool)(EQUAL(u256)(op1.tup0, op2.tup0) && EQUAL(u256)(op1.tup1, op2.tup1) && EQUAL(u256)(op1.tup2, op2.tup2));
}

static inline bool EQUAL(tuple_u256_u256_u256_u256)(struct tuple_u256_u256_u256_u256 op1, struct tuple_u256_u256_u256_u256 op2) {
  return (bool)(EQUAL(u256)(op1.tup0, op2.tup0) && EQUAL(u256)(op1.tup1, op2.tup1) && EQUAL(u256)(op1.tup2, op2.tup2) && EQUAL(u256)(op1.tup3, op2.tup3));
}

static inline bool EQUAL(LogTopics)(struct LogTopics op1, struct LogTopics op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_LogTopics1: {
    return EQUAL(u256)(op1.variants.LogTopics1, op2.variants.LogTopics1);
  }
  case Kind_LogTopics2: {
    return EQUAL(tuple_u256_u256)(op1.variants.LogTopics2, op2.variants.LogTopics2);
  }
  case Kind_LogTopics3: {
    return EQUAL(tuple_u256_u256_u256)(op1.variants.LogTopics3, op2.variants.LogTopics3);
  }
  case Kind_LogTopics4: {
    return EQUAL(tuple_u256_u256_u256_u256)(op1.variants.LogTopics4, op2.variants.LogTopics4);
  }
  case Kind_LogTopics0: {
    return true;
  }
  }
  return false;
}

static inline struct LogTopics LogTopics0(unit op) {
  struct LogTopics result;
  result.kind = Kind_LogTopics0;
  result.variants.LogTopics0 = op;
  return result;
}

static inline struct LogTopics LogTopics1(u256 op) {
  struct LogTopics result;
  result.kind = Kind_LogTopics1;
  result.variants.LogTopics1 = op;
  return result;
}

static inline struct LogTopics LogTopics2(struct tuple_u256_u256 op) {
  struct LogTopics result;
  result.kind = Kind_LogTopics2;
  result.variants.LogTopics2 = op;
  return result;
}

static inline struct LogTopics LogTopics3(struct tuple_u256_u256_u256 op) {
  struct LogTopics result;
  result.kind = Kind_LogTopics3;
  result.variants.LogTopics3 = op;
  return result;
}

static inline struct LogTopics LogTopics4(struct tuple_u256_u256_u256_u256 op) {
  struct LogTopics result;
  result.kind = Kind_LogTopics4;
  result.variants.LogTopics4 = op;
  return result;
}

static inline bool EQUAL(LogSeriesRef)(struct LogSeriesRef op1, struct LogSeriesRef op2) {
  return (bool)((op1.count == op2.count) && (op1.start == op2.start));
}

static inline bool EQUAL(ReceiptFields)(struct ReceiptFields op1, struct ReceiptFields op2) {
  return (bool)((op1.execution_gas == op2.execution_gas) && (op1.gas_used == op2.gas_used) && EQUAL(LogSeriesRef)(op1.logs, op2.logs) && (op1.state_gas == op2.state_gas) && EQUAL(bool)(op1.success, op2.success) && EQUAL(TxType)(op1.tx_type, op2.tx_type));
}

static inline bool EQUAL(BlobHashesFields)(struct BlobHashesFields op1, struct BlobHashesFields op2) {
  return (bool)(EQUAL(Bytes)(op1.bytes, op2.bytes) && (op1.count == op2.count));
}

static inline bool EQUAL(AuthorizationListRefFields)(struct AuthorizationListRefFields op1, struct AuthorizationListRefFields op2) {
  return (bool)((op1.count == op2.count) && EQUAL(Bytes)(op1.encoded, op2.encoded));
}

static inline bool EQUAL(Authorization)(struct Authorization op1, struct Authorization op2) {
  return (bool)(EQUAL(bytes20)(op1.address, op2.address) && EQUAL(bytes20)(op1.authority, op2.authority) && EQUAL(u256)(op1.chain_id, op2.chain_id) && (op1.nonce == op2.nonce) && EQUAL(bool)(op1.valid_sig, op2.valid_sig));
}

static inline bool EQUAL(AccessListRef)(struct AccessListRef op1, struct AccessListRef op2) {
  return (bool)((op1.address_count == op2.address_count) && EQUAL(Bytes)(op1.encoded, op2.encoded) && (op1.slot_count == op2.slot_count));
}

static inline bool EQUAL(TransactionFields)(struct TransactionFields op1, struct TransactionFields op2) {
  return (bool)(EQUAL(AccessListRef)(op1.access_list, op2.access_list) && EQUAL(AuthorizationListRefFields)(op1.authorizations, op2.authorizations) && EQUAL(BlobHashesFields)(op1.blob_hashes, op2.blob_hashes) && (op1.chain_id == op2.chain_id) && (op1.gas_limit == op2.gas_limit) && EQUAL(Bytes)(op1.input_src, op2.input_src) && EQUAL(bool)(op1.is_create, op2.is_create) && EQUAL(u256)(op1.max_blob_fee, op2.max_blob_fee) && EQUAL(u256)(op1.max_fee, op2.max_fee) && EQUAL(u256)(op1.max_priority_fee, op2.max_priority_fee) && EQUAL(u256)(op1.nonce, op2.nonce) && EQUAL(Bytes)(op1.pubkey, op2.pubkey) && EQUAL(Bytes)(op1.raw, op2.raw) && EQUAL(bytes20)(op1.recipient, op2.recipient) && EQUAL(bytes20)(op1.sender, op2.sender) && EQUAL(u256)(op1.sig_r, op2.sig_r) && EQUAL(u256)(op1.sig_s, op2.sig_s) && EQUAL(u256)(op1.sig_v, op2.sig_v) && EQUAL(bytes32)(op1.signing_hash, op2.signing_hash) && EQUAL(TxType)(op1.tx_type, op2.tx_type) && EQUAL(u256)(op1.value, op2.value));
}

static inline bool EQUAL(Withdrawal)(struct Withdrawal op1, struct Withdrawal op2) {
  return (bool)(EQUAL(bytes20)(op1.address, op2.address) && (op1.amount == op2.amount) && (op1.index == op2.index) && (op1.validator_index == op2.validator_index));
}


static inline bytes256 bytes256_zero(void) {
  bytes256 result = {{0}};
  return result;
}


static inline bool eq_bytes256(const bytes256 op1, const bytes256 op2) {
  return memcmp(op1.bytes, op2.bytes, 256) == 0;
}

static inline bytes256 internal_vector_init_bytes256(const int64_t length_arg) {
  (void)length_arg;
  return bytes256_zero();
}

static inline bytes256 internal_vector_update_bytes256(
    bytes256 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 256) {
    value.bytes[index] = (uint8_t)elem;
  }
  return value;
}

static inline uint64_t fast_vector_access_bytes256(const bytes256 value, const int64_t index) {
  return index >= 0 && index < 256 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline bytes256 fast_unsigned_vector_update_bytes256(
    bytes256 value, const uint64_t index, const uint64_t elem) {
  if (index < 256) {
    value.bytes[index] = (uint8_t)elem;
  }
  return value;
}

static inline uint64_t fast_unsigned_vector_access_bytes256(
    const bytes256 value, const uint64_t index) {
  return index < 256 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline bytes256 fast_vector_init_bytes256(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  bytes256 result = {0};
  for (size_t i = 0; i < 256; ++i) {
    result.bytes[i] = (uint8_t)elem;
  }
  return result;
}

static inline bytes256 fast_unsigned_vector_init_bytes256(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  bytes256 result = {0};
  for (size_t i = 0; i < 256; ++i) {
    result.bytes[i] = (uint8_t)elem;
  }
  return result;
}


static inline bool EQUAL(BlockHeader)(struct BlockHeader op1, struct BlockHeader op2) {
  return (bool)(EQUAL(u256)(op1.base_fee, op2.base_fee) && (op1.blob_gas_used == op2.blob_gas_used) && (op1.excess_blob_gas == op2.excess_blob_gas) && EQUAL(Bytes)(op1.extra_data, op2.extra_data) && EQUAL(bytes20)(op1.fee_recipient, op2.fee_recipient) && (op1.gas_limit == op2.gas_limit) && (op1.gas_used == op2.gas_used) && EQUAL(Bytes)(op1.logs_bloom, op2.logs_bloom) && (op1.number == op2.number) && EQUAL(bytes32)(op1.parent_beacon_block_root, op2.parent_beacon_block_root) && EQUAL(bytes32)(op1.parent_hash, op2.parent_hash) && EQUAL(u256)(op1.prev_randao, op2.prev_randao) && EQUAL(bytes32)(op1.receipts_root, op2.receipts_root) && (op1.slot_number == op2.slot_number) && EQUAL(bytes32)(op1.state_root, op2.state_root) && (op1.timestamp == op2.timestamp));
}

static inline bool EQUAL(BlockBody)(struct BlockBody op1, struct BlockBody op2) {
  return (bool)(EQUAL(Bytes)(op1.block_access_list, op2.block_access_list) && EQUAL(BoundedSszListRef)(op1.transactions, op2.transactions) && EQUAL(BoundedSszListRef)(op1.withdrawals, op2.withdrawals));
}

static inline bool EQUAL(Block)(struct Block op1, struct Block op2) {
  return (bool)(EQUAL(BlockBody)(op1.body, op2.body) && EQUAL(BlockHeader)(op1.header, op2.header));
}

static inline bool EQUAL(ExecutionPayload)(struct ExecutionPayload op1, struct ExecutionPayload op2) {
  return (bool)(EQUAL(Block)(op1.block, op2.block) && EQUAL(bytes32)(op1.expected_block_hash, op2.expected_block_hash));
}

static inline bool EQUAL(TxFrameGasSnapshotFields)(struct TxFrameGasSnapshotFields op1, struct TxFrameGasSnapshotFields op2) {
  return (bool)((op1.admitted_limit == op2.admitted_limit) && (op1.calldata_floor == op2.calldata_floor) && (op1.regular_limit == op2.regular_limit) && (op1.remaining == op2.remaining) && (op1.state_used == op2.state_used));
}

static inline bool EQUAL(TxFrameResultFields)(struct TxFrameResultFields op1, struct TxFrameResultFields op2) {
  return (bool)(EQUAL(TxFrameGasSnapshotFields)(op1.gas, op2.gas) && (op1.refund == op2.refund) && EQUAL(bool)(op1.success, op2.success));
}

static inline bool EQUAL(TransactionInitialGasFields)(struct TransactionInitialGasFields op1, struct TransactionInitialGasFields op2) {
  return (bool)((op1.admitted_limit == op2.admitted_limit) && (op1.calldata_floor == op2.calldata_floor) && (op1.execution_remaining == op2.execution_remaining) && (op1.intrinsic_execution == op2.intrinsic_execution) && (op1.intrinsic_state == op2.intrinsic_state) && (op1.regular_limit == op2.regular_limit) && (op1.state_remaining == op2.state_remaining));
}

static inline bool EQUAL(TxValidityFields)(struct TxValidityFields op1, struct TxValidityFields op2) {
  return (bool)(EQUAL(u256)(op1.blob_fee, op2.blob_fee) && EQUAL(TransactionInitialGasFields)(op1.gas, op2.gas) && EQUAL(u256)(op1.gas_price, op2.gas_price) && (op1.nonce_before == op2.nonce_before) && EQUAL(u256)(op1.priority_fee, op2.priority_fee) && EQUAL(bytes20)(op1.sender, op2.sender));
}

static inline bool EQUAL(TransactionGasAllowanceFields)(struct TransactionGasAllowanceFields op1, struct TransactionGasAllowanceFields op2) {
  return (bool)((op1.regular == op2.regular) && (op1.total == op2.total));
}

static inline bool EQUAL(Message)(struct Message op1, struct Message op2) {
  return (bool)(EQUAL(bytes20)(op1.address, op2.address) && EQUAL(bytes20)(op1.caller, op2.caller) && EQUAL(bytes20)(op1.code_address, op2.code_address) && (op1.depth == op2.depth) && EQUAL(bool)(op1.is_static, op2.is_static) && (op1.state_gas_reservoir == op2.state_gas_reservoir) && EQUAL(u256)(op1.value, op2.value));
}

static inline bool EQUAL(CreateKind)(enum CreateKind op1, enum CreateKind op2) {
  return (bool)(op1 == op2);
}

static inline enum CreateKind UNDEFINED(CreateKind)(void) { return CreateByNonce; }

static inline bool EQUAL(FrameCheckpoint)(struct FrameCheckpoint op1, struct FrameCheckpoint op2) {
  return (bool)((op1.call_depth == op2.call_depth) && EQUAL(CalldataSlice)(op1.calldata, op2.calldata) && EQUAL(CodeFields)(op1.code, op2.code) && (op1.gas_remaining == op2.gas_remaining) && EQUAL(Bytes)(op1.memory, op2.memory) && EQUAL(Message)(op1.message, op2.message) && (op1.pc == op2.pc) && (op1.refund == op2.refund) && EQUAL(fbits)(op1.stack_top, op2.stack_top) && (op1.state_gas_remaining == op2.state_gas_remaining) && (op1.state_gas_spilled == op2.state_gas_spilled) && EQUAL(FrameStatus)(op1.status, op2.status));
}

static inline bool EQUAL(CreateContinuation)(struct CreateContinuation op1, struct CreateContinuation op2) {
  return (bool)(EQUAL(bytes20)(op1.address, op2.address) && EQUAL(FrameCheckpoint)(op1.checkpoint, op2.checkpoint) && EQUAL(bool)(op1.new_account_charged, op2.new_account_charged));
}

static inline bool EQUAL(CallKind)(enum CallKind op1, enum CallKind op2) {
  return (bool)(op1 == op2);
}

static inline enum CallKind UNDEFINED(CallKind)(void) { return Call; }

static inline bool EQUAL(CallContinuation)(struct CallContinuation op1, struct CallContinuation op2) {
  return (bool)(EQUAL(FrameCheckpoint)(op1.checkpoint, op2.checkpoint) && EQUAL(bool)(op1.new_account_charged, op2.new_account_charged) && (op1.return_length == op2.return_length) && (op1.return_offset == op2.return_offset));
}

static inline bool EQUAL(FrameContinuation)(struct FrameContinuation op1, struct FrameContinuation op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_ResumeCall: {
    return EQUAL(CallContinuation)(op1.variants.ResumeCall, op2.variants.ResumeCall);
  }
  case Kind_ResumeCreate: {
    return EQUAL(CreateContinuation)(op1.variants.ResumeCreate, op2.variants.ResumeCreate);
  }
  case Kind_Empty: {
    return true;
  }
  }
  return false;
}

static inline struct FrameContinuation Empty(unit op) {
  struct FrameContinuation result;
  result.kind = Kind_Empty;
  result.variants.Empty = op;
  return result;
}

static inline struct FrameContinuation ResumeCall(struct CallContinuation op) {
  struct FrameContinuation result;
  result.kind = Kind_ResumeCall;
  result.variants.ResumeCall = op;
  return result;
}

static inline struct FrameContinuation ResumeCreate(struct CreateContinuation op) {
  struct FrameContinuation result;
  result.kind = Kind_ResumeCreate;
  result.variants.ResumeCreate = op;
  return result;
}

static inline bool EQUAL(TxEnvFields)(struct TxEnvFields op1, struct TxEnvFields op2) {
  return (bool)(EQUAL(BlobHashesFields)(op1.blob_hashes, op2.blob_hashes) && EQUAL(u256)(op1.gas_price, op2.gas_price) && EQUAL(bytes20)(op1.origin, op2.origin));
}

static inline bool EQUAL(WitnessContext)(struct WitnessContext op1, struct WitnessContext op2) {
  return (bool)(EQUAL(u256)(op1.parent_base_fee_per_gas, op2.parent_base_fee_per_gas) && (op1.parent_blob_gas_used == op2.parent_blob_gas_used) && (op1.parent_excess_blob_gas == op2.parent_excess_blob_gas) && EQUAL(bytes32)(op1.parent_hash, op2.parent_hash) && EQUAL(bytes32)(op1.parent_state_root, op2.parent_state_root));
}

static inline bool EQUAL(StatelessInput)(struct StatelessInput op1, struct StatelessInput op2) {
  return (bool)(EQUAL(ChainConfig)(op1.chain_config, op2.chain_config) && EQUAL(ExecutionPayload)(op1.payload, op2.payload));
}

static inline bool EQUAL(RlpEncoder)(struct RlpEncoder op1, struct RlpEncoder op2) {
  return (bool)((op1.expected_len == op2.expected_len) && (op1.start == op2.start));
}

static inline bool EQUAL(AccessListDecode)(struct AccessListDecode op1, struct AccessListDecode op2) {
  return (bool)((op1.address_count == op2.address_count) && (op1.slot_count == op2.slot_count));
}

static inline bool EQUAL(EnvField)(enum EnvField op1, enum EnvField op2) {
  return (bool)(op1 == op2);
}

static inline enum EnvField UNDEFINED(EnvField)(void) { return F_Number; }

static inline bool EQUAL(TransactionMergeSemantics)(struct TransactionMergeSemantics op1, struct TransactionMergeSemantics op2) {
  return (bool)(EQUAL(bool)(op1.delete_only_created, op2.delete_only_created) && EQUAL(bool)(op1.preserve_selfdestruct_balance, op2.preserve_selfdestruct_balance));
}

static inline bool EQUAL(tuple_u256_bits_64)(struct tuple_u256_bits_64 op1, struct tuple_u256_bits_64 op2) {
  return (bool)(EQUAL(u256)(op1.tup0, op2.tup0) && EQUAL(fbits)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_Bytes_Bytes)(struct tuple_Bytes_Bytes op1, struct tuple_Bytes_Bytes op2) {
  return (bool)(EQUAL(Bytes)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_Bytes_Bytes_1)(struct tuple_Bytes_Bytes_1 op1, struct tuple_Bytes_Bytes_1 op2) {
  return (bool)(EQUAL(Bytes)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_u256_Bytes)(struct tuple_u256_Bytes op1, struct tuple_u256_Bytes op2) {
  return (bool)(EQUAL(u256)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_bool_uint_64)(struct tuple_bool_uint_64 op1, struct tuple_bool_uint_64 op2) {
  return (bool)(EQUAL(bool)(op1.tup0, op2.tup0) && (op1.tup1 == op2.tup1));
}

static inline bool EQUAL(SstoreCosts)(struct SstoreCosts op1, struct SstoreCosts op2) {
  return (bool)((op1.execution == op2.execution) && (op1.refund == op2.refund) && (op1.state_charge == op2.state_charge) && (op1.state_credit == op2.state_credit));
}

static inline bool EQUAL(PrecompileResult)(struct PrecompileResult op1, struct PrecompileResult op2) {
  return (bool)(EQUAL(Bytes)(op1.output, op2.output) && EQUAL(bool)(op1.success, op2.success));
}

static inline bool EQUAL(tuple_uint_8_u256)(struct tuple_uint_8_u256 op1, struct tuple_uint_8_u256 op2) {
  return (bool)((op1.tup0 == op2.tup0) && EQUAL(u256)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(ast)(struct ast op1, struct ast op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_DUP: {
    return (bool)(op1.variants.DUP == op2.variants.DUP);
  }
  case Kind_DUPN: {
    return EQUAL(fbits)(op1.variants.DUPN, op2.variants.DUPN);
  }
  case Kind_EXCHANGE: {
    return EQUAL(fbits)(op1.variants.EXCHANGE, op2.variants.EXCHANGE);
  }
  case Kind_LOG: {
    return (bool)(op1.variants.LOG == op2.variants.LOG);
  }
  case Kind_PUSH: {
    return EQUAL(tuple_uint_8_u256)(op1.variants.PUSH, op2.variants.PUSH);
  }
  case Kind_SWAP: {
    return (bool)(op1.variants.SWAP == op2.variants.SWAP);
  }
  case Kind_SWAPN: {
    return EQUAL(fbits)(op1.variants.SWAPN, op2.variants.SWAPN);
  }
  case Kind_ADD:
  case Kind_ADDMOD:
  case Kind_ADDRESS:
  case Kind_AND:
  case Kind_BALANCE:
  case Kind_BASEFEE:
  case Kind_BLOBBASEFEE:
  case Kind_BLOBHASH:
  case Kind_BLOCKHASH:
  case Kind_BYTE:
  case Kind_CALL:
  case Kind_CALLCODE:
  case Kind_CALLDATACOPY:
  case Kind_CALLDATALOAD:
  case Kind_CALLDATASIZE:
  case Kind_CALLER:
  case Kind_CALLVALUE:
  case Kind_CHAINID:
  case Kind_CLZ:
  case Kind_CODECOPY:
  case Kind_CODESIZE:
  case Kind_COINBASE:
  case Kind_CREATE2:
  case Kind_DELEGATECALL:
  case Kind_DIV:
  case Kind_EQ:
  case Kind_EXP:
  case Kind_EXTCODECOPY:
  case Kind_EXTCODEHASH:
  case Kind_EXTCODESIZE:
  case Kind_GAS:
  case Kind_GASLIMIT:
  case Kind_GASPRICE:
  case Kind_GT:
  case Kind_INVALID:
  case Kind_ISZERO:
  case Kind_JUMP:
  case Kind_JUMPDEST:
  case Kind_JUMPI:
  case Kind_KECCAK256:
  case Kind_LT:
  case Kind_MCOPY:
  case Kind_MLOAD:
  case Kind_MOD:
  case Kind_MSIZE:
  case Kind_MSTORE:
  case Kind_MSTORE8:
  case Kind_MUL:
  case Kind_MULMOD:
  case Kind_NOT:
  case Kind_NUMBER:
  case Kind_OR:
  case Kind_ORIGIN:
  case Kind_PC:
  case Kind_POP:
  case Kind_PREVRANDAO:
  case Kind_RETURN:
  case Kind_RETURNDATACOPY:
  case Kind_RETURNDATASIZE:
  case Kind_REVERT:
  case Kind_SAR:
  case Kind_SDIV:
  case Kind_SELFBALANCE:
  case Kind_SELFDESTRUCT:
  case Kind_SGT:
  case Kind_SHL:
  case Kind_SHR:
  case Kind_SIGNEXTEND:
  case Kind_SLOAD:
  case Kind_SLOTNUM:
  case Kind_SLT:
  case Kind_SMOD:
  case Kind_SSTORE:
  case Kind_STATICCALL:
  case Kind_STOP:
  case Kind_SUB:
  case Kind_TIMESTAMP:
  case Kind_TLOAD:
  case Kind_TSTORE:
  case Kind_XOR:
  case Kind_opcode_CREATE: {
    return true;
  }
  }
  return false;
}

static inline struct ast ADD(unit op) {
  struct ast result;
  result.kind = Kind_ADD;
  result.variants.ADD = op;
  return result;
}

static inline struct ast ADDMOD(unit op) {
  struct ast result;
  result.kind = Kind_ADDMOD;
  result.variants.ADDMOD = op;
  return result;
}

static inline struct ast ADDRESS(unit op) {
  struct ast result;
  result.kind = Kind_ADDRESS;
  result.variants.ADDRESS = op;
  return result;
}

static inline struct ast AND(unit op) {
  struct ast result;
  result.kind = Kind_AND;
  result.variants.AND = op;
  return result;
}

static inline struct ast BALANCE(unit op) {
  struct ast result;
  result.kind = Kind_BALANCE;
  result.variants.BALANCE = op;
  return result;
}

static inline struct ast BASEFEE(unit op) {
  struct ast result;
  result.kind = Kind_BASEFEE;
  result.variants.BASEFEE = op;
  return result;
}

static inline struct ast BLOBBASEFEE(unit op) {
  struct ast result;
  result.kind = Kind_BLOBBASEFEE;
  result.variants.BLOBBASEFEE = op;
  return result;
}

static inline struct ast BLOBHASH(unit op) {
  struct ast result;
  result.kind = Kind_BLOBHASH;
  result.variants.BLOBHASH = op;
  return result;
}

static inline struct ast BLOCKHASH(unit op) {
  struct ast result;
  result.kind = Kind_BLOCKHASH;
  result.variants.BLOCKHASH = op;
  return result;
}

static inline struct ast BYTE(unit op) {
  struct ast result;
  result.kind = Kind_BYTE;
  result.variants.BYTE = op;
  return result;
}

static inline struct ast CALL(unit op) {
  struct ast result;
  result.kind = Kind_CALL;
  result.variants.CALL = op;
  return result;
}

static inline struct ast CALLCODE(unit op) {
  struct ast result;
  result.kind = Kind_CALLCODE;
  result.variants.CALLCODE = op;
  return result;
}

static inline struct ast CALLDATACOPY(unit op) {
  struct ast result;
  result.kind = Kind_CALLDATACOPY;
  result.variants.CALLDATACOPY = op;
  return result;
}

static inline struct ast CALLDATALOAD(unit op) {
  struct ast result;
  result.kind = Kind_CALLDATALOAD;
  result.variants.CALLDATALOAD = op;
  return result;
}

static inline struct ast CALLDATASIZE(unit op) {
  struct ast result;
  result.kind = Kind_CALLDATASIZE;
  result.variants.CALLDATASIZE = op;
  return result;
}

static inline struct ast CALLER(unit op) {
  struct ast result;
  result.kind = Kind_CALLER;
  result.variants.CALLER = op;
  return result;
}

static inline struct ast CALLVALUE(unit op) {
  struct ast result;
  result.kind = Kind_CALLVALUE;
  result.variants.CALLVALUE = op;
  return result;
}

static inline struct ast CHAINID(unit op) {
  struct ast result;
  result.kind = Kind_CHAINID;
  result.variants.CHAINID = op;
  return result;
}

static inline struct ast CLZ(unit op) {
  struct ast result;
  result.kind = Kind_CLZ;
  result.variants.CLZ = op;
  return result;
}

static inline struct ast CODECOPY(unit op) {
  struct ast result;
  result.kind = Kind_CODECOPY;
  result.variants.CODECOPY = op;
  return result;
}

static inline struct ast CODESIZE(unit op) {
  struct ast result;
  result.kind = Kind_CODESIZE;
  result.variants.CODESIZE = op;
  return result;
}

static inline struct ast COINBASE(unit op) {
  struct ast result;
  result.kind = Kind_COINBASE;
  result.variants.COINBASE = op;
  return result;
}

static inline struct ast CREATE2(unit op) {
  struct ast result;
  result.kind = Kind_CREATE2;
  result.variants.CREATE2 = op;
  return result;
}

static inline struct ast DELEGATECALL(unit op) {
  struct ast result;
  result.kind = Kind_DELEGATECALL;
  result.variants.DELEGATECALL = op;
  return result;
}

static inline struct ast DIV(unit op) {
  struct ast result;
  result.kind = Kind_DIV;
  result.variants.DIV = op;
  return result;
}

static inline struct ast DUP(uint8_t op) {
  struct ast result;
  result.kind = Kind_DUP;
  result.variants.DUP = op;
  return result;
}

static inline struct ast DUPN(uint64_t op) {
  struct ast result;
  result.kind = Kind_DUPN;
  result.variants.DUPN = op;
  return result;
}

static inline struct ast EQ(unit op) {
  struct ast result;
  result.kind = Kind_EQ;
  result.variants.EQ = op;
  return result;
}

static inline struct ast EXCHANGE(uint64_t op) {
  struct ast result;
  result.kind = Kind_EXCHANGE;
  result.variants.EXCHANGE = op;
  return result;
}

static inline struct ast EXP(unit op) {
  struct ast result;
  result.kind = Kind_EXP;
  result.variants.EXP = op;
  return result;
}

static inline struct ast EXTCODECOPY(unit op) {
  struct ast result;
  result.kind = Kind_EXTCODECOPY;
  result.variants.EXTCODECOPY = op;
  return result;
}

static inline struct ast EXTCODEHASH(unit op) {
  struct ast result;
  result.kind = Kind_EXTCODEHASH;
  result.variants.EXTCODEHASH = op;
  return result;
}

static inline struct ast EXTCODESIZE(unit op) {
  struct ast result;
  result.kind = Kind_EXTCODESIZE;
  result.variants.EXTCODESIZE = op;
  return result;
}

static inline struct ast GAS(unit op) {
  struct ast result;
  result.kind = Kind_GAS;
  result.variants.GAS = op;
  return result;
}

static inline struct ast GASLIMIT(unit op) {
  struct ast result;
  result.kind = Kind_GASLIMIT;
  result.variants.GASLIMIT = op;
  return result;
}

static inline struct ast GASPRICE(unit op) {
  struct ast result;
  result.kind = Kind_GASPRICE;
  result.variants.GASPRICE = op;
  return result;
}

static inline struct ast GT(unit op) {
  struct ast result;
  result.kind = Kind_GT;
  result.variants.GT = op;
  return result;
}

static inline struct ast INVALID(unit op) {
  struct ast result;
  result.kind = Kind_INVALID;
  result.variants.INVALID = op;
  return result;
}

static inline struct ast ISZERO(unit op) {
  struct ast result;
  result.kind = Kind_ISZERO;
  result.variants.ISZERO = op;
  return result;
}

static inline struct ast JUMP(unit op) {
  struct ast result;
  result.kind = Kind_JUMP;
  result.variants.JUMP = op;
  return result;
}

static inline struct ast JUMPDEST(unit op) {
  struct ast result;
  result.kind = Kind_JUMPDEST;
  result.variants.JUMPDEST = op;
  return result;
}

static inline struct ast JUMPI(unit op) {
  struct ast result;
  result.kind = Kind_JUMPI;
  result.variants.JUMPI = op;
  return result;
}

static inline struct ast KECCAK256(unit op) {
  struct ast result;
  result.kind = Kind_KECCAK256;
  result.variants.KECCAK256 = op;
  return result;
}

static inline struct ast LOG(uint8_t op) {
  struct ast result;
  result.kind = Kind_LOG;
  result.variants.LOG = op;
  return result;
}

static inline struct ast LT(unit op) {
  struct ast result;
  result.kind = Kind_LT;
  result.variants.LT = op;
  return result;
}

static inline struct ast MCOPY(unit op) {
  struct ast result;
  result.kind = Kind_MCOPY;
  result.variants.MCOPY = op;
  return result;
}

static inline struct ast MLOAD(unit op) {
  struct ast result;
  result.kind = Kind_MLOAD;
  result.variants.MLOAD = op;
  return result;
}

static inline struct ast MOD(unit op) {
  struct ast result;
  result.kind = Kind_MOD;
  result.variants.MOD = op;
  return result;
}

static inline struct ast MSIZE(unit op) {
  struct ast result;
  result.kind = Kind_MSIZE;
  result.variants.MSIZE = op;
  return result;
}

static inline struct ast MSTORE(unit op) {
  struct ast result;
  result.kind = Kind_MSTORE;
  result.variants.MSTORE = op;
  return result;
}

static inline struct ast MSTORE8(unit op) {
  struct ast result;
  result.kind = Kind_MSTORE8;
  result.variants.MSTORE8 = op;
  return result;
}

static inline struct ast MUL(unit op) {
  struct ast result;
  result.kind = Kind_MUL;
  result.variants.MUL = op;
  return result;
}

static inline struct ast MULMOD(unit op) {
  struct ast result;
  result.kind = Kind_MULMOD;
  result.variants.MULMOD = op;
  return result;
}

static inline struct ast NOT(unit op) {
  struct ast result;
  result.kind = Kind_NOT;
  result.variants.NOT = op;
  return result;
}

static inline struct ast NUMBER(unit op) {
  struct ast result;
  result.kind = Kind_NUMBER;
  result.variants.NUMBER = op;
  return result;
}

static inline struct ast OR(unit op) {
  struct ast result;
  result.kind = Kind_OR;
  result.variants.OR = op;
  return result;
}

static inline struct ast ORIGIN(unit op) {
  struct ast result;
  result.kind = Kind_ORIGIN;
  result.variants.ORIGIN = op;
  return result;
}

static inline struct ast PC(unit op) {
  struct ast result;
  result.kind = Kind_PC;
  result.variants.PC = op;
  return result;
}

static inline struct ast POP(unit op) {
  struct ast result;
  result.kind = Kind_POP;
  result.variants.POP = op;
  return result;
}

static inline struct ast PREVRANDAO(unit op) {
  struct ast result;
  result.kind = Kind_PREVRANDAO;
  result.variants.PREVRANDAO = op;
  return result;
}

static inline struct ast PUSH(struct tuple_uint_8_u256 op) {
  struct ast result;
  result.kind = Kind_PUSH;
  result.variants.PUSH = op;
  return result;
}

static inline struct ast RETURN(unit op) {
  struct ast result;
  result.kind = Kind_RETURN;
  result.variants.RETURN = op;
  return result;
}

static inline struct ast RETURNDATACOPY(unit op) {
  struct ast result;
  result.kind = Kind_RETURNDATACOPY;
  result.variants.RETURNDATACOPY = op;
  return result;
}

static inline struct ast RETURNDATASIZE(unit op) {
  struct ast result;
  result.kind = Kind_RETURNDATASIZE;
  result.variants.RETURNDATASIZE = op;
  return result;
}

static inline struct ast REVERT(unit op) {
  struct ast result;
  result.kind = Kind_REVERT;
  result.variants.REVERT = op;
  return result;
}

static inline struct ast SAR(unit op) {
  struct ast result;
  result.kind = Kind_SAR;
  result.variants.SAR = op;
  return result;
}

static inline struct ast SDIV(unit op) {
  struct ast result;
  result.kind = Kind_SDIV;
  result.variants.SDIV = op;
  return result;
}

static inline struct ast SELFBALANCE(unit op) {
  struct ast result;
  result.kind = Kind_SELFBALANCE;
  result.variants.SELFBALANCE = op;
  return result;
}

static inline struct ast SELFDESTRUCT(unit op) {
  struct ast result;
  result.kind = Kind_SELFDESTRUCT;
  result.variants.SELFDESTRUCT = op;
  return result;
}

static inline struct ast SGT(unit op) {
  struct ast result;
  result.kind = Kind_SGT;
  result.variants.SGT = op;
  return result;
}

static inline struct ast SHL(unit op) {
  struct ast result;
  result.kind = Kind_SHL;
  result.variants.SHL = op;
  return result;
}

static inline struct ast SHR(unit op) {
  struct ast result;
  result.kind = Kind_SHR;
  result.variants.SHR = op;
  return result;
}

static inline struct ast SIGNEXTEND(unit op) {
  struct ast result;
  result.kind = Kind_SIGNEXTEND;
  result.variants.SIGNEXTEND = op;
  return result;
}

static inline struct ast SLOAD(unit op) {
  struct ast result;
  result.kind = Kind_SLOAD;
  result.variants.SLOAD = op;
  return result;
}

static inline struct ast SLOTNUM(unit op) {
  struct ast result;
  result.kind = Kind_SLOTNUM;
  result.variants.SLOTNUM = op;
  return result;
}

static inline struct ast SLT(unit op) {
  struct ast result;
  result.kind = Kind_SLT;
  result.variants.SLT = op;
  return result;
}

static inline struct ast SMOD(unit op) {
  struct ast result;
  result.kind = Kind_SMOD;
  result.variants.SMOD = op;
  return result;
}

static inline struct ast SSTORE(unit op) {
  struct ast result;
  result.kind = Kind_SSTORE;
  result.variants.SSTORE = op;
  return result;
}

static inline struct ast STATICCALL(unit op) {
  struct ast result;
  result.kind = Kind_STATICCALL;
  result.variants.STATICCALL = op;
  return result;
}

static inline struct ast STOP(unit op) {
  struct ast result;
  result.kind = Kind_STOP;
  result.variants.STOP = op;
  return result;
}

static inline struct ast SUB(unit op) {
  struct ast result;
  result.kind = Kind_SUB;
  result.variants.SUB = op;
  return result;
}

static inline struct ast SWAP(uint8_t op) {
  struct ast result;
  result.kind = Kind_SWAP;
  result.variants.SWAP = op;
  return result;
}

static inline struct ast SWAPN(uint64_t op) {
  struct ast result;
  result.kind = Kind_SWAPN;
  result.variants.SWAPN = op;
  return result;
}

static inline struct ast TIMESTAMP(unit op) {
  struct ast result;
  result.kind = Kind_TIMESTAMP;
  result.variants.TIMESTAMP = op;
  return result;
}

static inline struct ast TLOAD(unit op) {
  struct ast result;
  result.kind = Kind_TLOAD;
  result.variants.TLOAD = op;
  return result;
}

static inline struct ast TSTORE(unit op) {
  struct ast result;
  result.kind = Kind_TSTORE;
  result.variants.TSTORE = op;
  return result;
}

static inline struct ast XOR(unit op) {
  struct ast result;
  result.kind = Kind_XOR;
  result.variants.XOR = op;
  return result;
}

static inline struct ast opcode_CREATE(unit op) {
  struct ast result;
  result.kind = Kind_opcode_CREATE;
  result.variants.opcode_CREATE = op;
  return result;
}

static inline bool EQUAL(tuple_uint_16_uint_16)(struct tuple_uint_16_uint_16 op1, struct tuple_uint_16_uint_16 op2) {
  return (bool)((op1.tup0 == op2.tup0) && (op1.tup1 == op2.tup1));
}

static inline bool EQUAL(tuple_uint_32_uint_64)(struct tuple_uint_32_uint_64 op1, struct tuple_uint_32_uint_64 op2) {
  return (bool)((op1.tup0 == op2.tup0) && (op1.tup1 == op2.tup1));
}

static inline bool EQUAL(tuple_LogTopics_bits_64)(struct tuple_LogTopics_bits_64 op1, struct tuple_LogTopics_bits_64 op2) {
  return (bool)(EQUAL(LogTopics)(op1.tup0, op2.tup0) && EQUAL(fbits)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_bits_64_uint_64)(struct tuple_bits_64_uint_64 op1, struct tuple_bits_64_uint_64 op2) {
  return (bool)(EQUAL(fbits)(op1.tup0, op2.tup0) && (op1.tup1 == op2.tup1));
}

static inline bool EQUAL(tuple_bits_64_Bytes_uint_64)(struct tuple_bits_64_Bytes_uint_64 op1, struct tuple_bits_64_Bytes_uint_64 op2) {
  return (bool)(EQUAL(fbits)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1) && (op1.tup2 == op2.tup2));
}

static inline bool EQUAL(tuple_uint_32_bits_64_uint_64)(struct tuple_uint_32_bits_64_uint_64 op1, struct tuple_uint_32_bits_64_uint_64 op2) {
  return (bool)((op1.tup0 == op2.tup0) && EQUAL(fbits)(op1.tup1, op2.tup1) && (op1.tup2 == op2.tup2));
}

static inline bool EQUAL(tuple_uint_32_bits_64_Bytes_uint_64)(struct tuple_uint_32_bits_64_Bytes_uint_64 op1, struct tuple_uint_32_bits_64_Bytes_uint_64 op2) {
  return (bool)((op1.tup0 == op2.tup0) && EQUAL(fbits)(op1.tup1, op2.tup1) && EQUAL(Bytes)(op1.tup2, op2.tup2) && (op1.tup3 == op2.tup3));
}

static inline bool EQUAL(CreateSemantics)(struct CreateSemantics op1, struct CreateSemantics op2) {
  return EQUAL(bool)(op1.uses_salt, op2.uses_salt);
}

static inline bool EQUAL(CallSemantics)(struct CallSemantics op1, struct CallSemantics op2) {
  return (bool)(EQUAL(bool)(op1.enters_static_context, op2.enters_static_context) && EQUAL(bool)(op1.inherits_caller_and_value, op2.inherits_caller_and_value) && EQUAL(bool)(op1.takes_value, op2.takes_value) && EQUAL(bool)(op1.transfers_value, op2.transfers_value) && EQUAL(bool)(op1.uses_target_address, op2.uses_target_address));
}

static inline bool EQUAL(tuple_uint_8_bits_64_Bytes_uint_64)(struct tuple_uint_8_bits_64_Bytes_uint_64 op1, struct tuple_uint_8_bits_64_Bytes_uint_64 op2) {
  return (bool)((op1.tup0 == op2.tup0) && EQUAL(fbits)(op1.tup1, op2.tup1) && EQUAL(Bytes)(op1.tup2, op2.tup2) && (op1.tup3 == op2.tup3));
}

static inline bool EQUAL(TxUpfrontResult)(struct TxUpfrontResult op1, struct TxUpfrontResult op2) {
  return (bool)((op1.authorization_refund == op2.authorization_refund) && EQUAL(bool)(op1.create_target_prestate_empty, op2.create_target_prestate_empty));
}

static inline bool EQUAL(TransactionPreparation)(struct TransactionPreparation op1, struct TransactionPreparation op2) {
  return (bool)(EQUAL(bool)(op1.delegated, op2.delegated) && EQUAL(bool)(op1.ready, op2.ready));
}

static inline bool EQUAL(TransactionCosts)(struct TransactionCosts op1, struct TransactionCosts op2) {
  return (bool)(EQUAL(u256)(op1.blob_fee, op2.blob_fee) && (op1.blob_gas == op2.blob_gas) && (op1.calldata_floor == op2.calldata_floor) && (op1.intrinsic_execution == op2.intrinsic_execution) && (op1.intrinsic_state == op2.intrinsic_state) && EQUAL(u256)(op1.upfront, op2.upfront));
}

static inline bool EQUAL(IntrinsicGasCost)(struct IntrinsicGasCost op1, struct IntrinsicGasCost op2) {
  return (bool)((op1.calldata_floor == op2.calldata_floor) && (op1.execution == op2.execution) && (op1.state == op2.state));
}

static inline bool EQUAL(TriePath)(struct TriePath op1, struct TriePath op2) {
  return (bool)(EQUAL(bytes32)(op1.data, op2.data) && (op1.len == op2.len));
}

static inline bool EQUAL(tuple_bool_TriePath)(struct tuple_bool_TriePath op1, struct tuple_bool_TriePath op2) {
  return (bool)(EQUAL(bool)(op1.tup0, op2.tup0) && EQUAL(TriePath)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(TrieLeafValue)(struct TrieLeafValue op1, struct TrieLeafValue op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_InputTrieLeaf: {
    return EQUAL(Bytes)(op1.variants.InputTrieLeaf, op2.variants.InputTrieLeaf);
  }
  case Kind_ScratchTrieLeaf: {
    return EQUAL(Bytes)(op1.variants.ScratchTrieLeaf, op2.variants.ScratchTrieLeaf);
  }
  }
  return false;
}

static inline struct TrieLeafValue InputTrieLeaf(Bytes op) {
  struct TrieLeafValue result;
  result.kind = Kind_InputTrieLeaf;
  result.variants.InputTrieLeaf = op;
  return result;
}

static inline struct TrieLeafValue ScratchTrieLeaf(Bytes op) {
  struct TrieLeafValue result;
  result.kind = Kind_ScratchTrieLeaf;
  result.variants.ScratchTrieLeaf = op;
  return result;
}

static inline bool EQUAL(InlineNode)(struct InlineNode op1, struct InlineNode op2) {
  return (bool)(EQUAL(bytes32)(op1.data, op2.data) && (op1.len == op2.len));
}

static inline bool EQUAL(NodeRef)(struct NodeRef op1, struct NodeRef op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_HashRef: {
    return EQUAL(bytes32)(op1.variants.HashRef, op2.variants.HashRef);
  }
  case Kind_InputInlineRef: {
    return EQUAL(Bytes)(op1.variants.InputInlineRef, op2.variants.InputInlineRef);
  }
  case Kind_ScratchInlineRef: {
    return EQUAL(InlineNode)(op1.variants.ScratchInlineRef, op2.variants.ScratchInlineRef);
  }
  case Kind_EmptyRef: {
    return true;
  }
  }
  return false;
}

static inline struct NodeRef EmptyRef(unit op) {
  struct NodeRef result;
  result.kind = Kind_EmptyRef;
  result.variants.EmptyRef = op;
  return result;
}

static inline struct NodeRef HashRef(bytes32 op) {
  struct NodeRef result;
  result.kind = Kind_HashRef;
  result.variants.HashRef = op;
  return result;
}

static inline struct NodeRef InputInlineRef(Bytes op) {
  struct NodeRef result;
  result.kind = Kind_InputInlineRef;
  result.variants.InputInlineRef = op;
  return result;
}

static inline struct NodeRef ScratchInlineRef(struct InlineNode op) {
  struct NodeRef result;
  result.kind = Kind_ScratchInlineRef;
  result.variants.ScratchInlineRef = op;
  return result;
}

static inline bool EQUAL(tuple_TriePath_NodeRef)(struct tuple_TriePath_NodeRef op1, struct tuple_TriePath_NodeRef op2) {
  return (bool)(EQUAL(TriePath)(op1.tup0, op2.tup0) && EQUAL(NodeRef)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_TriePath_Bytes)(struct tuple_TriePath_Bytes op1, struct tuple_TriePath_Bytes op2) {
  return (bool)(EQUAL(TriePath)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_vector_16_NodeRef_Bytes)(struct tuple_vector_16_NodeRef_Bytes op1, struct tuple_vector_16_NodeRef_Bytes op2) {
  return (bool)(EQUAL(vector_16_NodeRef)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(ScratchTrieNode)(struct ScratchTrieNode op1, struct ScratchTrieNode op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_ScratchBranchNode: {
    return EQUAL(tuple_vector_16_NodeRef_Bytes)(op1.variants.ScratchBranchNode, op2.variants.ScratchBranchNode);
  }
  case Kind_ScratchExtensionNode: {
    return EQUAL(tuple_TriePath_NodeRef)(op1.variants.ScratchExtensionNode, op2.variants.ScratchExtensionNode);
  }
  case Kind_ScratchLeafNode: {
    return EQUAL(tuple_TriePath_Bytes)(op1.variants.ScratchLeafNode, op2.variants.ScratchLeafNode);
  }
  }
  return false;
}

static inline struct ScratchTrieNode ScratchBranchNode(struct tuple_vector_16_NodeRef_Bytes op) {
  struct ScratchTrieNode result;
  result.kind = Kind_ScratchBranchNode;
  result.variants.ScratchBranchNode = op;
  return result;
}

static inline struct ScratchTrieNode ScratchExtensionNode(struct tuple_TriePath_NodeRef op) {
  struct ScratchTrieNode result;
  result.kind = Kind_ScratchExtensionNode;
  result.variants.ScratchExtensionNode = op;
  return result;
}

static inline struct ScratchTrieNode ScratchLeafNode(struct tuple_TriePath_Bytes op) {
  struct ScratchTrieNode result;
  result.kind = Kind_ScratchLeafNode;
  result.variants.ScratchLeafNode = op;
  return result;
}

static inline bool EQUAL(tuple_TriePath_Bytes_1)(struct tuple_TriePath_Bytes_1 op1, struct tuple_TriePath_Bytes_1 op2) {
  return (bool)(EQUAL(TriePath)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_vector_16_NodeRef_Bytes_1)(struct tuple_vector_16_NodeRef_Bytes_1 op1, struct tuple_vector_16_NodeRef_Bytes_1 op2) {
  return (bool)(EQUAL(vector_16_NodeRef)(op1.tup0, op2.tup0) && EQUAL(Bytes)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(InputTrieNode)(struct InputTrieNode op1, struct InputTrieNode op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_InputBranchNode: {
    return EQUAL(tuple_vector_16_NodeRef_Bytes_1)(op1.variants.InputBranchNode, op2.variants.InputBranchNode);
  }
  case Kind_InputExtensionNode: {
    return EQUAL(tuple_TriePath_NodeRef)(op1.variants.InputExtensionNode, op2.variants.InputExtensionNode);
  }
  case Kind_InputLeafNode: {
    return EQUAL(tuple_TriePath_Bytes_1)(op1.variants.InputLeafNode, op2.variants.InputLeafNode);
  }
  }
  return false;
}

static inline struct InputTrieNode InputBranchNode(struct tuple_vector_16_NodeRef_Bytes_1 op) {
  struct InputTrieNode result;
  result.kind = Kind_InputBranchNode;
  result.variants.InputBranchNode = op;
  return result;
}

static inline struct InputTrieNode InputExtensionNode(struct tuple_TriePath_NodeRef op) {
  struct InputTrieNode result;
  result.kind = Kind_InputExtensionNode;
  result.variants.InputExtensionNode = op;
  return result;
}

static inline struct InputTrieNode InputLeafNode(struct tuple_TriePath_Bytes_1 op) {
  struct InputTrieNode result;
  result.kind = Kind_InputLeafNode;
  result.variants.InputLeafNode = op;
  return result;
}

static inline bool EQUAL(TrieUpdateSource)(struct TrieUpdateSource op1, struct TrieUpdateSource op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_StorageTrieUpdates: {
    return EQUAL(bytes20)(op1.variants.StorageTrieUpdates, op2.variants.StorageTrieUpdates);
  }
  case Kind_ChangedAccountTrieUpdates: {
    return true;
  }
  }
  return false;
}

static inline struct TrieUpdateSource ChangedAccountTrieUpdates(unit op) {
  struct TrieUpdateSource result;
  result.kind = Kind_ChangedAccountTrieUpdates;
  result.variants.ChangedAccountTrieUpdates = op;
  return result;
}

static inline struct TrieUpdateSource StorageTrieUpdates(bytes20 op) {
  struct TrieUpdateSource result;
  result.kind = Kind_StorageTrieUpdates;
  result.variants.StorageTrieUpdates = op;
  return result;
}

static inline bool EQUAL(TrieUpdateRelation)(struct TrieUpdateRelation op1, struct TrieUpdateRelation op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_UpdateBeyondPrefix: {
    return (bool)(op1.variants.UpdateBeyondPrefix == op2.variants.UpdateBeyondPrefix);
  }
  case Kind_UpdateUnderPrefix: {
    return EQUAL(TriePath)(op1.variants.UpdateUnderPrefix, op2.variants.UpdateUnderPrefix);
  }
  case Kind_UpdateSourceExhausted: {
    return true;
  }
  }
  return false;
}

static inline struct TrieUpdateRelation UpdateBeyondPrefix(uint8_t op) {
  struct TrieUpdateRelation result;
  result.kind = Kind_UpdateBeyondPrefix;
  result.variants.UpdateBeyondPrefix = op;
  return result;
}

static inline struct TrieUpdateRelation UpdateSourceExhausted(unit op) {
  struct TrieUpdateRelation result;
  result.kind = Kind_UpdateSourceExhausted;
  result.variants.UpdateSourceExhausted = op;
  return result;
}

static inline struct TrieUpdateRelation UpdateUnderPrefix(struct TriePath op) {
  struct TrieUpdateRelation result;
  result.kind = Kind_UpdateUnderPrefix;
  result.variants.UpdateUnderPrefix = op;
  return result;
}

static inline bool EQUAL(TrieChange)(struct TrieChange op1, struct TrieChange op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_TriePut: {
    return EQUAL(Bytes)(op1.variants.TriePut, op2.variants.TriePut);
  }
  case Kind_TrieDelete: {
    return true;
  }
  }
  return false;
}

static inline struct TrieChange TrieDelete(unit op) {
  struct TrieChange result;
  result.kind = Kind_TrieDelete;
  result.variants.TrieDelete = op;
  return result;
}

static inline struct TrieChange TriePut(Bytes op) {
  struct TrieChange result;
  result.kind = Kind_TriePut;
  result.variants.TriePut = op;
  return result;
}

static inline bool EQUAL(TrieUpdate)(struct TrieUpdate op1, struct TrieUpdate op2) {
  return (bool)(EQUAL(TrieChange)(op1.change, op2.change) && EQUAL(TriePath)(op1.key, op2.key));
}

static inline bool EQUAL(TrieUpdateFetch)(struct TrieUpdateFetch op1, struct TrieUpdateFetch op2) {
  return (bool)(EQUAL(bool)(op1.available, op2.available) && EQUAL(TrieUpdate)(op1.update, op2.update));
}

static inline bool EQUAL(TrieUpdateCursor)(struct TrieUpdateCursor op1, struct TrieUpdateCursor op2) {
  return (bool)(EQUAL(TrieUpdate)(op1.current, op2.current) && EQUAL(TrieUpdateRelation)(op1.relation, op2.relation) && EQUAL(TrieUpdateSource)(op1.source, op2.source));
}

static inline bool EQUAL(TrieItemValue)(struct TrieItemValue op1, struct TrieItemValue op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_BranchItem: {
    return EQUAL(NodeRef)(op1.variants.BranchItem, op2.variants.BranchItem);
  }
  case Kind_LeafItem: {
    return EQUAL(TrieLeafValue)(op1.variants.LeafItem, op2.variants.LeafItem);
  }
  case Kind_SubtreeItem: {
    return EQUAL(NodeRef)(op1.variants.SubtreeItem, op2.variants.SubtreeItem);
  }
  case Kind_EmptySubtree: {
    return true;
  }
  }
  return false;
}

static inline struct TrieItemValue BranchItem(struct NodeRef op) {
  struct TrieItemValue result;
  result.kind = Kind_BranchItem;
  result.variants.BranchItem = op;
  return result;
}

static inline struct TrieItemValue EmptySubtree(unit op) {
  struct TrieItemValue result;
  result.kind = Kind_EmptySubtree;
  result.variants.EmptySubtree = op;
  return result;
}

static inline struct TrieItemValue LeafItem(struct TrieLeafValue op) {
  struct TrieItemValue result;
  result.kind = Kind_LeafItem;
  result.variants.LeafItem = op;
  return result;
}

static inline struct TrieItemValue SubtreeItem(struct NodeRef op) {
  struct TrieItemValue result;
  result.kind = Kind_SubtreeItem;
  result.variants.SubtreeItem = op;
  return result;
}

static inline bool EQUAL(TrieItem)(struct TrieItem op1, struct TrieItem op2) {
  return (bool)(EQUAL(TriePath)(op1.path, op2.path) && EQUAL(TrieItemValue)(op1.value, op2.value));
}

static inline bool EQUAL(TrieChildren)(struct TrieChildren op1, struct TrieChildren op2) {
  return (bool)(EQUAL(vector_16_NodeRef)(op1.children, op2.children) && (op1.count == op2.count) && EQUAL(fbits)(op1.mask, op2.mask) && EQUAL(TrieItem)(op1.only, op2.only));
}

static inline bool EQUAL(tuple_TrieUpdate_TrieUpdateCursor)(struct tuple_TrieUpdate_TrieUpdateCursor op1, struct tuple_TrieUpdate_TrieUpdateCursor op2) {
  return (bool)(EQUAL(TrieUpdate)(op1.tup0, op2.tup0) && EQUAL(TrieUpdateCursor)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_TrieItem_TrieUpdateCursor)(struct tuple_TrieItem_TrieUpdateCursor op1, struct tuple_TrieItem_TrieUpdateCursor op2) {
  return (bool)(EQUAL(TrieItem)(op1.tup0, op2.tup0) && EQUAL(TrieUpdateCursor)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(tuple_TrieUpdate_bool)(struct tuple_TrieUpdate_bool op1, struct tuple_TrieUpdate_bool op2) {
  return (bool)(EQUAL(TrieUpdate)(op1.tup0, op2.tup0) && EQUAL(bool)(op1.tup1, op2.tup1));
}

static inline bool EQUAL(TrieRootResult)(struct TrieRootResult op1, struct TrieRootResult op2) {
  return (bool)(EQUAL(bool)(op1.changed, op2.changed) && EQUAL(bytes32)(op1.root, op2.root));
}

static inline bool EQUAL(ParentHeaderFields)(struct ParentHeaderFields op1, struct ParentHeaderFields op2) {
  return (bool)(EQUAL(u256)(op1.base_fee, op2.base_fee) && (op1.blob_gas_used == op2.blob_gas_used) && (op1.excess_blob_gas == op2.excess_blob_gas) && EQUAL(bool)(op1.have_base_fee, op2.have_base_fee) && EQUAL(bool)(op1.have_blob_gas, op2.have_blob_gas) && EQUAL(bool)(op1.have_excess_blob_gas, op2.have_excess_blob_gas) && EQUAL(bool)(op1.have_parent, op2.have_parent) && EQUAL(bool)(op1.have_state, op2.have_state) && EQUAL(bytes32)(op1.parent_hash, op2.parent_hash) && EQUAL(bytes32)(op1.state_root, op2.state_root));
}

static inline bool EQUAL(WitnessHeaderIndex)(struct WitnessHeaderIndex op1, struct WitnessHeaderIndex op2) {
  return (bool)(EQUAL(BoundedSszListCursor)(op1.cursor, op2.cursor) && EQUAL(u256)(op1.parent_base_fee_per_gas, op2.parent_base_fee_per_gas) && (op1.parent_blob_gas_used == op2.parent_blob_gas_used) && (op1.parent_excess_blob_gas == op2.parent_excess_blob_gas) && EQUAL(bool)(op1.parent_fields_valid, op2.parent_fields_valid) && EQUAL(bytes32)(op1.parent_state_root, op2.parent_state_root) && EQUAL(bytes32)(op1.previous_hash, op2.previous_hash) && EQUAL(bool)(op1.valid, op2.valid));
}

static inline bool EQUAL(StatelessInputRef)(struct StatelessInputRef op1, struct StatelessInputRef op2) {
  return (bool)(EQUAL(Bytes)(op1.block_access_list, op2.block_access_list) && EQUAL(Bytes)(op1.builder_deposit_requests, op2.builder_deposit_requests) && EQUAL(Bytes)(op1.builder_exit_requests, op2.builder_exit_requests) && EQUAL(Bytes)(op1.chain_config, op2.chain_config) && EQUAL(Bytes)(op1.consolidation_requests, op2.consolidation_requests) && EQUAL(Bytes)(op1.deposits, op2.deposits) && EQUAL(Bytes)(op1.execution_payload, op2.execution_payload) && EQUAL(Bytes)(op1.extra_data, op2.extra_data) && EQUAL(Bytes)(op1.new_payload_request, op2.new_payload_request) && EQUAL(ProtocolProfileFields)(op1.protocol, op2.protocol) && EQUAL(Bytes)(op1.public_keys, op2.public_keys) && EQUAL(BoundedSszListRef)(op1.transactions, op2.transactions) && EQUAL(Bytes)(op1.versioned_hashes, op2.versioned_hashes) && EQUAL(Bytes)(op1.withdrawal_requests, op2.withdrawal_requests) && EQUAL(BoundedSszListRef)(op1.withdrawals, op2.withdrawals) && EQUAL(BoundedSszListRef)(op1.witness_codes, op2.witness_codes) && EQUAL(BoundedSszListRef)(op1.witness_headers, op2.witness_headers) && EQUAL(BoundedSszListRef)(op1.witness_state, op2.witness_state));
}

static inline bool EQUAL(BlockGasUsageFields)(struct BlockGasUsageFields op1, struct BlockGasUsageFields op2) {
  return (bool)((op1.execution == op2.execution) && EQUAL(u128)(op1.receipts, op2.receipts) && (op1.state == op2.state));
}

static inline bool EQUAL(BlockExecutionResult)(struct BlockExecutionResult op1, struct BlockExecutionResult op2) {
  return (bool)((op1.blob_gas_used == op2.blob_gas_used) && (op1.execution_gas_used == op2.execution_gas_used) && EQUAL(bytes20)(op1.first_tx_recipient, op2.first_tx_recipient) && (op1.header_gas_used == op2.header_gas_used) && EQUAL(LogSeriesRef)(op1.logs, op2.logs) && EQUAL(bytes32)(op1.receipts_root, op2.receipts_root) && (op1.state_gas_used == op2.state_gas_used));
}

static inline bool EQUAL(HtrRequestKind)(enum HtrRequestKind op1, enum HtrRequestKind op2) {
  return (bool)(op1 == op2);
}

static inline enum HtrRequestKind UNDEFINED(HtrRequestKind)(void) { return HtrDeposit; }static inline vector_32_bits_8 internal_vector_update_vector_32_bits_8(vector_32_bits_8 op, const int64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  op.data[m] = elem;
  return op;
}

static inline vector_32_bits_8 internal_vector_init_vector_32_bits_8(const int64_t len) {
  vector_32_bits_8 rop;
  rop.len = (size_t)len;
  return rop;
}

static inline vector_16_NodeRef fast_unsigned_vector_init_vector_16_NodeRef(const uint64_t n, struct NodeRef elem) {
  vector_16_NodeRef vec;
  size_t m = (size_t)n;
  vec.len = m;
  for (size_t i = 0; i < m; ++i) {
    vec.data[i] = elem;
  }
  return vec;
}

static inline struct NodeRef fast_vector_access_vector_16_NodeRef(vector_16_NodeRef op, int64_t n) {
  return op.data[(size_t)n];
}

static inline vector_16_NodeRef fast_unsigned_vector_update_vector_16_NodeRef(vector_16_NodeRef op, const uint64_t n, struct NodeRef elem) {
  size_t m = (size_t)n;
  op.data[m] = elem;
  return op;
}

static inline bool EQUAL(vector_16_NodeRef)(const vector_16_NodeRef op1, const vector_16_NodeRef op2) {
  if (op1.len != op2.len) {
    return false;
  }
  bool result = true;
  for (size_t i = 0; i < op1.len; ++i) {
    result &= EQUAL(NodeRef)(op1.data[i], op2.data[i]);
  }
  return result;
}
