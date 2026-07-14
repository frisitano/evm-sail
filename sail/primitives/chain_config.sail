/*md
# Chain configuration

The decoded `SszChainConfig`: the chain id and the active fork — both as
the raw SSZ `ProtocolFork` index and as the [Fork][type-Fork] whose
execution rules apply (blob-parameter-only forks collapse to their base) —
plus whether the fork's activation point has been reached. Pure data — no
registers, no externs.
*/

/*! A fork's blob parameters (EIP-4844/EIP-7691): target and maximum blob
    counts per block, and the base-fee update fraction. */
struct BlobSchedule = {
    target : int,
    max : int,
    base_fee_update_fraction : int,
}

/*! The decoded chain configuration for the executing payload. */
struct ChainConfig = {
    chain_id : int,
    fork_index : int, /* raw SSZ ProtocolFork index                     */
    fork : Fork, /* execution rules for that index                 */
    activation_active : bool, /* the fork's activation point is reached         */
    blob_schedule : option(BlobSchedule),
    blob_schedule_shape_valid : bool,
}

/*! Structural equality of blob schedules. */
function blob_schedule_equal(a : BlobSchedule, b : BlobSchedule) -> bool =
    (a.target == b.target) & (a.max == b.max) & (a.base_fee_update_fraction == b.base_fee_update_fraction)

/*! The protocol-defined blob schedule for an SSZ `ProtocolFork` index:
    Cancun 3/6, Prague/Osaka 6/9, BPO1 10/15, BPO2+ 14/21; forks before
    Cancun have none. */
function expected_blob_schedule(idx : int) -> option(BlobSchedule) =
    if idx < 15 then {
        None()
    } else {
        if idx == 15 then {
            Some(struct { target = 3, max = 6, base_fee_update_fraction = 3338477 })
        } else {
            if idx <= 17 then {
                Some(struct { target = 6, max = 9, base_fee_update_fraction = 5007716 })
            } else {
                if idx == 18 then {
                    Some(struct { target = 10, max = 15, base_fee_update_fraction = 8346193 })
                } else {
                    Some(struct { target = 14, max = 21, base_fee_update_fraction = 11684671 })
                }
            }
        }
    }

/*! Whether the config's blob schedule is well-shaped and equals the
    protocol-defined schedule for its fork index. */
function chain_config_blob_schedule_valid(cc : ChainConfig) -> bool = {
    if not_bool(cc.blob_schedule_shape_valid) then {
        false
    } else {
        match (expected_blob_schedule(cc.fork_index), cc.blob_schedule) {
            (None(), None()) => true,
            (Some(expected), Some(actual)) => blob_schedule_equal(expected, actual),
            _ => false,
        }
    }
}
