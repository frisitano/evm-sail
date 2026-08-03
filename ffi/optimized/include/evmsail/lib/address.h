#ifndef EVMSAIL_OPTIMIZED_LIB_ADDRESS_H
#define EVMSAIL_OPTIMIZED_LIB_ADDRESS_H

#include "evmsail/prelude.h"

Address create2_address(
    Address sender, U256 salt,
    Hash32 init_hash);

#endif
