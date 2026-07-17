/* ======================================================================== */
/* PRIMITIVES: CHAIN CONFIG                                                 */
/*                                                                          */
/* The decoded SszChainConfig: the chain id, the active fork -- BOTH as the */
/* raw SSZ ProtocolFork index and as the Fork whose execution rules apply   */
/* (difficulty-delay and blob-parameter-only forks collapse to their base)  */
/* -- and whether the fork's activation point has been reached.             */
/* been reached. Pure data -- no registers, no externs.                     */
/* ======================================================================== */

struct BlobSchedule = {
    target : blob_count,
    max : blob_count,
    base_fee_update_fraction : blob_fee_update_fraction,
}

struct ChainConfig = {
    chain_id : chain_identifier,
    fork_index : protocol_fork_index, /* raw SSZ ProtocolFork index            */
    fork : Fork, /* execution rules for that index                 */
    activation_active : bool, /* EELS _is_activation_active for this payload    */
    blob_schedule : option(BlobSchedule),
    blob_schedule_shape_valid : bool,
}

function blob_schedule_equal(a : BlobSchedule, b : BlobSchedule) -> bool =
    (a.target == b.target) & (a.max == b.max) & (a.base_fee_update_fraction == b.base_fee_update_fraction)

function expected_blob_schedule(idx : protocol_fork_index) -> option(BlobSchedule) =
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
