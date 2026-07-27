/* Generated-aggregate glue for the suspended-frame continuation stack.
 * Storage is reserved without initialization on the first push, so a guest
 * that never enters a child frame pays no allocation cost and deeper calls
 * require no reallocation or copying. The optimized ABI is inline. The
 * standard ABI owns the GMP-backed fields directly and initializes only the
 * slots it reaches. */
#include EVMSAIL_MODEL_H
#include "frame_stack.h"

#include <stdint.h>
#include <stdlib.h>

#define FRAME_STACK_CAPACITY UINT64_C(1024)

static struct zFrameContinuation *frame_slots;
static uint64_t frame_top;
#ifdef EVMSAIL_STANDARD_ABI
static uint64_t frame_initialized;

static void frame_int_clone(sail_int *dst, const sail_int src) {
  CREATE(sail_int)(dst);
  COPY(sail_int)(dst, src);
}

static void frame_slice_clone(struct zByteSliceFields *dst,
                              const struct zByteSliceFields *src) {
  *dst = *src;
  frame_int_clone(&dst->zlen, src->zlen);
  frame_int_clone(&dst->zoff, src->zoff);
}

static void frame_slice_drop(struct zByteSliceFields *slice) {
  KILL(sail_int)(&slice->zlen);
  KILL(sail_int)(&slice->zoff);
}

static void frame_status_clone(struct zFrameStatus *dst,
                               const struct zFrameStatus *src) {
  *dst = *src;
  if (src->kind != Kind_zHalted) return;

  switch (src->variants.zHalted.kind) {
  case Kind_zHaltReturn:
    frame_slice_clone(&dst->variants.zHalted.variants.zHaltReturn,
                      &src->variants.zHalted.variants.zHaltReturn);
    break;
  case Kind_zHaltRevert:
    frame_slice_clone(&dst->variants.zHalted.variants.zHaltRevert,
                      &src->variants.zHalted.variants.zHaltRevert);
    break;
  default:
    break;
  }
}

static void frame_status_drop(struct zFrameStatus *status) {
  if (status->kind != Kind_zHalted) return;

  switch (status->variants.zHalted.kind) {
  case Kind_zHaltReturn:
    frame_slice_drop(&status->variants.zHalted.variants.zHaltReturn);
    break;
  case Kind_zHaltRevert:
    frame_slice_drop(&status->variants.zHalted.variants.zHaltRevert);
    break;
  default:
    break;
  }
}

static void frame_checkpoint_clone(struct zFrameCheckpoint *dst,
                                   const struct zFrameCheckpoint *src) {
  *dst = *src;
  frame_slice_clone(&dst->zcalldata, &src->zcalldata);
  frame_slice_clone(&dst->zcode.zbytes, &src->zcode.zbytes);
  frame_int_clone(&dst->zgas_remaining, src->zgas_remaining);
  frame_slice_clone(&dst->zmemory, &src->zmemory);
  frame_int_clone(&dst->zmessage.zstate_gas_reservoir,
                  src->zmessage.zstate_gas_reservoir);
  frame_int_clone(&dst->zpc, src->zpc);
  frame_int_clone(&dst->zstate, src->zstate);
  frame_int_clone(&dst->zstate_gas_remaining, src->zstate_gas_remaining);
  frame_status_clone(&dst->zstatus, &src->zstatus);
}

static void frame_checkpoint_drop(struct zFrameCheckpoint *checkpoint) {
  frame_slice_drop(&checkpoint->zcalldata);
  frame_slice_drop(&checkpoint->zcode.zbytes);
  KILL(sail_int)(&checkpoint->zgas_remaining);
  frame_slice_drop(&checkpoint->zmemory);
  KILL(sail_int)(&checkpoint->zmessage.zstate_gas_reservoir);
  KILL(sail_int)(&checkpoint->zpc);
  KILL(sail_int)(&checkpoint->zstate);
  KILL(sail_int)(&checkpoint->zstate_gas_remaining);
  frame_status_drop(&checkpoint->zstatus);
}

static void frame_continuation_drop(struct zFrameContinuation *continuation) {
  switch (continuation->kind) {
  case Kind_zResumeCall:
    frame_checkpoint_drop(
        &continuation->variants.zResumeCall.zcheckpoint);
    KILL(sail_int)(&continuation->variants.zResumeCall.zreturn_length);
    KILL(sail_int)(&continuation->variants.zResumeCall.zreturn_offset);
    break;
  case Kind_zResumeCreate:
    frame_checkpoint_drop(
        &continuation->variants.zResumeCreate.zcheckpoint);
    break;
  default:
    break;
  }
}

static void frame_continuation_clone(struct zFrameContinuation *dst,
                                     const struct zFrameContinuation *src) {
  frame_continuation_drop(dst);
  *dst = *src;

  switch (src->kind) {
  case Kind_zResumeCall:
    frame_checkpoint_clone(&dst->variants.zResumeCall.zcheckpoint,
                           &src->variants.zResumeCall.zcheckpoint);
    frame_int_clone(&dst->variants.zResumeCall.zreturn_length,
                    src->variants.zResumeCall.zreturn_length);
    frame_int_clone(&dst->variants.zResumeCall.zreturn_offset,
                    src->variants.zResumeCall.zreturn_offset);
    break;
  case Kind_zResumeCreate:
    frame_checkpoint_clone(&dst->variants.zResumeCreate.zcheckpoint,
                           &src->variants.zResumeCreate.zcheckpoint);
    break;
  default:
    break;
  }
}
#endif

static void frame_stack_ensure(void) {
  if (frame_slots != NULL) return;
  frame_slots =
      malloc((size_t)FRAME_STACK_CAPACITY * sizeof(*frame_slots));
  if (frame_slots == NULL) abort();
}

unit frame_stack_reset(unit u) {
  (void)u;
  frame_top = 0;
  return UNIT;
}

unit frame_stack_push(struct zFrameContinuation continuation) {
  if (frame_top >= FRAME_STACK_CAPACITY) abort();
  frame_stack_ensure();
#ifdef EVMSAIL_STANDARD_ABI
  if (frame_top == frame_initialized) {
    frame_slots[frame_top].kind = Kind_zEmpty;
    frame_initialized++;
  }
  frame_continuation_clone(&frame_slots[frame_top], &continuation);
  frame_top++;
#else
  frame_slots[frame_top++] = continuation;
#endif
  return UNIT;
}

void frame_stack_pop(struct zFrameContinuation *out, unit u) {
  (void)u;
  if (frame_top == 0) {
#ifndef EVMSAIL_STANDARD_ABI
    out->kind = Kind_zEmpty;
#endif
    return;
  }
  if (frame_slots == NULL) abort();
#ifdef EVMSAIL_STANDARD_ABI
  frame_top--;
  frame_continuation_clone(out, &frame_slots[frame_top]);
#else
  *out = frame_slots[--frame_top];
#endif
}
