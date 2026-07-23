
/*md
# Chain configuration

The decoded `SszChainConfig`. Its activation point is validated while decoding,
so the resulting value only carries the chain id. The schema prefix selects the
protocol profile, so neither the fork identifier nor its blob schedule is
repeated inside the SSZ body. Pure data — no registers, no externs.
*/
/*! A fork's blob parameters (EIP-4844/EIP-7691): target and maximum blob
    counts per block, and the base-fee update fraction. */
struct BlobSchedule = {
    target : blob_target_count,
    max : blob_count,
    base_fee_update_fraction : blob_fee_update_fraction,
}

/*! Execution rules and protocol constants selected by the stable fork byte
    in the stateless-input schema identifier. */
struct ProtocolProfile = {
    fork : Fork,
    blob_schedule : BlobSchedule,
}

/*! The inactive blob schedule used before EIP-4844. */
let NO_BLOB_SCHEDULE : BlobSchedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 }

/*! Resolves the schema's stable `ProtocolFork` byte to the execution-rule
    family and its protocol-defined blob schedule. BPO forks share Osaka
    execution rules but retain their distinct blob parameters. An unknown
    input byte is rejected before any profile field reaches a kernel
    register. */
function protocol_profile(schema_fork : byte) -> ProtocolProfile =
    if schema_fork == 0x0a then {
        struct { fork = Berlin, blob_schedule = NO_BLOB_SCHEDULE }
    } else {
        if (schema_fork == 0x0b) | (schema_fork == 0x0c) | (schema_fork == 0x0d) then {
            struct { fork = London, blob_schedule = NO_BLOB_SCHEDULE }
        } else {
            if schema_fork == 0x0e then {
                struct { fork = Paris, blob_schedule = NO_BLOB_SCHEDULE }
            } else {
                if schema_fork == 0x0f then {
                    struct { fork = Shanghai, blob_schedule = NO_BLOB_SCHEDULE }
                } else {
                    if schema_fork == 0x10 then {
                        struct {
                            fork = Cancun,
                            blob_schedule = struct { target = 3, max = 6, base_fee_update_fraction = 3338477 },
                        }
                    } else {
                        if (schema_fork == 0x11) | (schema_fork == 0x12) then {
                            struct {
                                fork = if schema_fork == 0x11 then { Prague } else { Osaka },
                                blob_schedule = struct { target = 6, max = 9, base_fee_update_fraction = 5007716 },
                            }
                        } else {
                            if schema_fork == 0x13 then {
                                struct {
                                    fork = Osaka,
                                    blob_schedule = struct { target = 10, max = 15, base_fee_update_fraction = 8346193 },
                                }
                            } else {
                                if schema_fork == 0x14 then {
                                    struct {
                                        fork = Osaka,
                                        blob_schedule =
                                            struct { target = 14, max = 21, base_fee_update_fraction = 11684671 },
                                    }
                                } else {
                                    if schema_fork == 0x15 then {
                                        struct {
                                            fork = Amsterdam,
                                            blob_schedule =
                                                struct { target = 14, max = 21, base_fee_update_fraction = 11684671 },
                                        }
                                    } else {
                                        throw InvalidBlock(InvalidConfig)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

/*! The decoded chain configuration for the executing payload. */
struct ChainConfig = {
    chain_id : chain_identifier,
}
