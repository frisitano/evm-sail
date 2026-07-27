#ifndef EVMSAIL_OPTIMIZED_RESULT_H
#define EVMSAIL_OPTIMIZED_RESULT_H

/*
 * Result constructors for optimized whole-operation C entry points.
 *
 * C reports protocol failures as explicit generated union values. The thin
 * Sail wrappers that consume these values own the corresponding
 * `throw InvalidBlock(reason)`, keeping exception semantics visible to Sail's
 * ordinary effect inference and to extraction backends.
 *
 * Include this header only after the generated model header, which owns every
 * aggregate layout and BlockError.
 */
static inline void evmsail_unit_result_ok(
    struct zOptimizzedUnitResult *result) {
  result->kind = Kind_zOptimizzedUnitOk;
  result->variants.zOptimizzedUnitOk = UNIT;
}

static inline void evmsail_unit_result_error(
    struct zOptimizzedUnitResult *result, enum zBlockError reason) {
  result->kind = Kind_zOptimizzedUnitError;
  result->variants.zOptimizzedUnitError = reason;
}

static inline void evmsail_hash_result_ok(
    struct zOptimizzedHashResult *result, sail_hash value) {
  result->kind = Kind_zOptimizzedHashOk;
  result->variants.zOptimizzedHashOk = value;
}

static inline void evmsail_hash_result_error(
    struct zOptimizzedHashResult *result, enum zBlockError reason) {
  result->kind = Kind_zOptimizzedHashError;
  result->variants.zOptimizzedHashError = reason;
}

static inline void evmsail_account_result_ok(
    struct zOptimizzedAccountResult *result, struct zAccount value) {
  result->kind = Kind_zOptimizzedAccountOk;
  result->variants.zOptimizzedAccountOk = value;
}

static inline void evmsail_account_result_error(
    struct zOptimizzedAccountResult *result, enum zBlockError reason) {
  result->kind = Kind_zOptimizzedAccountError;
  result->variants.zOptimizzedAccountError = reason;
}

static inline void evmsail_storage_result_ok(
    struct zOptimizzedStorageResult *result, struct zStorageValue value) {
  result->kind = Kind_zOptimizzedStorageOk;
  result->variants.zOptimizzedStorageOk = value;
}

static inline void evmsail_storage_result_error(
    struct zOptimizzedStorageResult *result, enum zBlockError reason) {
  result->kind = Kind_zOptimizzedStorageError;
  result->variants.zOptimizzedStorageError = reason;
}

#endif
