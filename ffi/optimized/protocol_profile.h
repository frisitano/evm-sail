#ifndef EVMSAIL_OPTIMIZED_PROTOCOL_PROFILE_H
#define EVMSAIL_OPTIMIZED_PROTOCOL_PROFILE_H

#include <stdint.h>

/*
 * Fork is a Sail integer range, so optimized C compares the profile field
 * against the same ordered integer values. This is deliberately not a second
 * C enum: the active value remains the integer stored by ProtocolProfile.
 */
#define EVMSAIL_FORK_LONDON UINT64_C(6)
#define EVMSAIL_FORK_SHANGHAI UINT64_C(10)
#define EVMSAIL_FORK_CANCUN UINT64_C(11)
#define EVMSAIL_FORK_PRAGUE UINT64_C(12)
#define EVMSAIL_FORK_OSAKA UINT64_C(13)
#define EVMSAIL_FORK_AMSTERDAM UINT64_C(16)

static inline uint64_t evmsail_active_fork(void) {
  return zk_execution_profile.zprotocol.zfork;
}

#endif
