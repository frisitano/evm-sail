#pragma once

#include "evmsail/spec/host/accelerators.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct ScratchRlpFieldRef
struct ScratchRlpFieldRef {
  uint32_t content_len;
  bool is_list;
  Bytes source;
};

// struct RlpFieldRef
struct RlpFieldRef {
  uint32_t content_len;
  bool is_list;
  Bytes source;
};


#ifdef __cplusplus
}
#endif
