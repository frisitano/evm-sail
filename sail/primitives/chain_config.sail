
/*md
# Chain configuration

The protocol profile the guest executes under: Amsterdam with its blob
schedule, fixed by the model. The stateless input's schema id admits only this
profile and its SSZ body carries just the chain id. Pure data — no registers,
no externs.
*/
/*! The base-fee update fractions used by some supported blob schedule. */
type blob_fee_update_fraction_parameter('denominator : Int) -> Bool =
       'denominator == inactive_blob_fee_update_fraction
    |  'denominator == cancun_blob_fee_update_fraction
    |  'denominator == prague_blob_fee_update_fraction
    |  'denominator == bpo1_blob_fee_update_fraction
    |  'denominator == bpo2_blob_fee_update_fraction

/*! The five protocol-defined blob schedules. Keeping the fields related
    excludes cross-products of individually valid constants which are not a
    schedule used by any fork. */
type blob_schedule_parameters(
    'target : Int,
    'maximum : Int,
    'denominator : Int,
) -> Bool =
    blob_fee_update_fraction_parameter('denominator)
    & (
       (
           'target == blob_schedule_inactive_count
           & 'maximum == blob_schedule_inactive_count
           & 'denominator == inactive_blob_fee_update_fraction
       )
    |  (
           'target == cancun_blob_target_count
           & 'maximum == cancun_blob_max_count
           & 'denominator == cancun_blob_fee_update_fraction
       )
    |  (
           'target == prague_blob_target_count
           & 'maximum == prague_blob_max_count
           & 'denominator == prague_blob_fee_update_fraction
       )
    |  (
           'target == bpo1_blob_target_count
           & 'maximum == bpo1_blob_max_count
           & 'denominator == bpo1_blob_fee_update_fraction
       )
    |  (
           'target == bpo2_blob_target_count
           & 'maximum == bpo2_blob_max_count
           & 'denominator == bpo2_blob_fee_update_fraction
       )
    )

/*! A fork's associated blob parameters (EIP-4844/EIP-7691): target and
    maximum blob counts per block, and the base-fee update fraction. The
    existential indices retain the selected schedule's equations wherever the
    value is consumed. */
struct BlobScheduleFields('target : Int, 'maximum : Int, 'denominator : Int),
    blob_schedule_parameters('target, 'maximum, 'denominator) = {
    target : int('target),
    max : int('maximum),
    base_fee_update_fraction : int('denominator),
}

/*! An admitted blob schedule with its three parameters packed
    existentially. */
type BlobSchedule = {
    'target 'maximum 'denominator,
    blob_schedule_parameters('target, 'maximum, 'denominator).
    BlobScheduleFields('target, 'maximum, 'denominator)
}

/*! Exact active-fork schedule types. These aliases make a profile refinement
    visible at call sites without reconstructing or copying its schedule. */
type cancun_blob_schedule = BlobScheduleFields(
    cancun_blob_target_count,
    cancun_blob_max_count,
    cancun_blob_fee_update_fraction,
)
/*! The exact schedule type of the Prague fork (EIP-7691). */
type prague_blob_schedule = BlobScheduleFields(
    prague_blob_target_count,
    prague_blob_max_count,
    prague_blob_fee_update_fraction,
)
/*! The exact schedule type of the Osaka fork, which retains Prague's blob
    parameters. */
type osaka_blob_schedule = BlobScheduleFields(
    prague_blob_target_count,
    prague_blob_max_count,
    prague_blob_fee_update_fraction,
)
/*! The exact schedule type of the first blob-parameter-only fork. */
type bpo1_blob_schedule = BlobScheduleFields(
    bpo1_blob_target_count,
    bpo1_blob_max_count,
    bpo1_blob_fee_update_fraction,
)
/*! The exact schedule type of the second blob-parameter-only fork. */
type bpo2_blob_schedule = BlobScheduleFields(
    bpo2_blob_target_count,
    bpo2_blob_max_count,
    bpo2_blob_fee_update_fraction,
)
/*! The exact schedule type of the Amsterdam fork, which retains BPO2's blob
    parameters. */
type amsterdam_blob_schedule = BlobScheduleFields(
    bpo2_blob_target_count,
    bpo2_blob_max_count,
    bpo2_blob_fee_update_fraction,
)

/*! Constructs one admitted schedule while retaining its three singleton
    indices. */
val blob_schedule : forall 'target 'maximum 'denominator, blob_schedule_parameters('target, 'maximum, 'denominator). (
        int('target),
        int('maximum),
        int('denominator),
    ) -> BlobScheduleFields('target, 'maximum, 'denominator)
function blob_schedule(target, maximum, denominator) =
    struct { target = target, max = maximum, base_fee_update_fraction = denominator }

/*! The protocol parameter tuples admitted by supported forks. Keeping the
    fields under one constraint preserves their relationships: consumers know
    that they received one real protocol profile rather than an arbitrary
    cross-product of individually valid constants. */
type protocol_profile_parameters(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
    'code_limit : Int,
    'initcode_limit : Int,
    'transaction_total_gas_limit : Int,
    'transaction_regular_gas_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
) -> Bool =
       (
           'fork == berlin_fork_value
           & 'target == blob_schedule_inactive_count
           & 'maximum == blob_schedule_inactive_count
           & 'denominator == inactive_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == inactive_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == ssz_uint_bound
           & 'transaction_blob_limit == blob_schedule_inactive_count
           & 'refund_divisor == pre_london_refund_divisor
       )
    |  (
           (
               london_fork_value <= 'fork
               & 'fork <= paris_fork_value
           )
           & 'target == blob_schedule_inactive_count
           & 'maximum == blob_schedule_inactive_count
           & 'denominator == inactive_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == inactive_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == ssz_uint_bound
           & 'transaction_blob_limit == blob_schedule_inactive_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == shanghai_fork_value
           & 'target == blob_schedule_inactive_count
           & 'maximum == blob_schedule_inactive_count
           & 'denominator == inactive_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == ssz_uint_bound
           & 'transaction_blob_limit == blob_schedule_inactive_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == first_blob_fork_value
           & 'target == cancun_blob_target_count
           & 'maximum == cancun_blob_max_count
           & 'denominator == cancun_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == ssz_uint_bound
           & 'transaction_blob_limit == cancun_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == prague_fork_value
           & 'target == prague_blob_target_count
           & 'maximum == prague_blob_max_count
           & 'denominator == prague_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == ssz_uint_bound
           & 'transaction_blob_limit == prague_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == osaka_fork_value
           & 'target == prague_blob_target_count
           & 'maximum == prague_blob_max_count
           & 'denominator == prague_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_regular_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_blob_limit == cancun_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == bpo1_fork_value
           & 'target == bpo1_blob_target_count
           & 'maximum == bpo1_blob_max_count
           & 'denominator == bpo1_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_regular_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_blob_limit == cancun_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == bpo2_fork_value
           & 'target == bpo2_blob_target_count
           & 'maximum == bpo2_blob_max_count
           & 'denominator == bpo2_blob_fee_update_fraction
           & 'code_limit == pre_amsterdam_deployed_code_size_limit
           & 'initcode_limit == pre_amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_regular_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_blob_limit == cancun_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )
    |  (
           'fork == amsterdam_fork_value
           & 'target == bpo2_blob_target_count
           & 'maximum == bpo2_blob_max_count
           & 'denominator == bpo2_blob_fee_update_fraction
           & 'code_limit == amsterdam_deployed_code_size_limit
           & 'initcode_limit == amsterdam_initcode_size_limit
           & 'transaction_total_gas_limit == ssz_uint_bound
           & 'transaction_regular_gas_limit == eip7825_transaction_gas_limit
           & 'transaction_blob_limit == cancun_blob_max_count
           & 'refund_divisor == post_london_refund_divisor
       )

/*! The reachable excess-blob-gas ceiling derived from a correlated profile.
    The type-level equation is also the field's singleton type, so the stored
    value cannot disagree with its fork or schedule. */
type profile_excess_blob_gas_limit(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
) = int(
    if 'fork < first_blob_fork_value then
        0
    else
          blob_fee_word_exponent_limit * 'denominator
        + ('maximum - 'target) * gas_per_blob_value
)

/*! Computes [profile_excess_blob_gas_limit][] from a profile's concrete
    fork and schedule values. */
val compute_profile_excess_blob_gas_limit : forall 'fork 'target 'maximum 'denominator,
                                                 0
                                              <= 'fork
                                              &  'fork
                                              <= amsterdam_fork_value
                                              &  blob_schedule_parameters('target, 'maximum, 'denominator). (
        int('fork),
        int('target),
        int('maximum),
        int('denominator),
    ) -> profile_excess_blob_gas_limit('fork, 'target, 'maximum, 'denominator)
function compute_profile_excess_blob_gas_limit(fork, target, maximum, denominator) =
    if fork < Cancun then {
        0
    } else {
        sizeof(blob_fee_word_exponent_limit) * denominator + (maximum - target) * sizeof(gas_per_blob_value)
    }

/*! Execution rules and computation bounds selected together by one schema
    fork. The singleton indices retain the exact selected parameter tuple. */
struct ProtocolProfileFields(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
    'code_limit : Int,
    'initcode_limit : Int,
    'transaction_total_gas_limit : Int,
    'transaction_regular_gas_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
), protocol_profile_parameters(
    'fork,
    'target,
    'maximum,
    'denominator,
    'code_limit,
    'initcode_limit,
    'transaction_total_gas_limit,
    'transaction_regular_gas_limit,
    'transaction_blob_limit,
    'refund_divisor,
) = {
    fork : int('fork),
    blob_schedule : BlobScheduleFields('target, 'maximum, 'denominator),
    excess_blob_gas_limit : profile_excess_blob_gas_limit(
        'fork,
        'target,
        'maximum,
        'denominator,
    ),
    deployed_code_size_limit : int('code_limit),
    initcode_size_limit : int('initcode_limit),
    transaction_total_gas_limit : int('transaction_total_gas_limit),
    transaction_regular_gas_limit : int('transaction_regular_gas_limit),
    transaction_blob_limit : int('transaction_blob_limit),
    refund_divisor : int('refund_divisor),
}

/*! A protocol profile with its parameter tuple packed existentially;
    unpacking recovers the admitted combination's equations. */
type ProtocolProfile = {
    'fork
    'target
    'maximum
    'denominator
    'code_limit
    'initcode_limit
    'transaction_total_gas_limit
    'transaction_regular_gas_limit
    'transaction_blob_limit
    'refund_divisor,
    protocol_profile_parameters(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'transaction_total_gas_limit,
        'transaction_regular_gas_limit,
        'transaction_blob_limit,
        'refund_divisor,
    ).
    ProtocolProfileFields(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'transaction_total_gas_limit,
        'transaction_regular_gas_limit,
        'transaction_blob_limit,
        'refund_divisor,
    )
}

val pack_protocol_profile : forall 'fork 'target 'maximum 'denominator 'code_limit 'initcode_limit 'transaction_total_gas_limit 'transaction_regular_gas_limit 'transaction_blob_limit 'refund_divisor,
                              protocol_profile_parameters(
                                  'fork,
                                  'target,
                                  'maximum,
                                  'denominator,
                                  'code_limit,
                                  'initcode_limit,
                                  'transaction_total_gas_limit,
                                  'transaction_regular_gas_limit,
                                  'transaction_blob_limit,
                                  'refund_divisor,
                              ). ProtocolProfileFields(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'transaction_total_gas_limit,
        'transaction_regular_gas_limit,
        'transaction_blob_limit,
        'refund_divisor,
    ) -> ProtocolProfile
function pack_protocol_profile(profile) = profile

/*! The gas limits which become concrete once a correlated protocol profile
    is paired with the executing block header. `transaction_total_limit`
    bounds the transaction's complete reservoir; `transaction_regular_limit`
    bounds the regular-execution part of that reservoir. Receipt accumulation
    is indexed directly by the two block reservoirs and therefore needs no
    duplicate scalar limit here. */
type gas_limits_parameters(
    'block_limit : Int,
    'profile_total_limit : Int,
    'profile_regular_limit : Int,
    'transaction_total_limit : Int,
    'transaction_regular_limit : Int,
) -> Bool =
       0 <= 'block_limit
    &  'block_limit <= block_gas_limit_bound
    &  protocol_transaction_total_gas_limit_value('profile_total_limit)
    &  protocol_transaction_regular_gas_limit_value('profile_regular_limit)
    &  'transaction_total_limit
        == (if 'block_limit < 'profile_total_limit then
            'block_limit
        else
            'profile_total_limit)
    &  'transaction_regular_limit
        == (if 'transaction_total_limit < 'profile_regular_limit then
            'transaction_total_limit
        else
            'profile_regular_limit)

/*! The concrete gas ceilings for the executing block: the header's block
    limit, the derived per-transaction total and regular limits, and the
    fixed system-call limits. */
struct GasLimitsFields(
    'block_limit : Int,
    'profile_total_limit : Int,
    'profile_regular_limit : Int,
    'transaction_total_limit : Int,
    'transaction_regular_limit : Int,
), gas_limits_parameters(
    'block_limit,
    'profile_total_limit,
    'profile_regular_limit,
    'transaction_total_limit,
    'transaction_regular_limit,
) = {
    block_limit : int('block_limit),
    transaction_total_limit : int('transaction_total_limit),
    transaction_regular_limit : int('transaction_regular_limit),
    system_regular_limit : int(30000000),
    system_state_limit : int(0),
}

/*! Gas limits with their five indices packed existentially. */
type GasLimits = {
    'block_limit
    'profile_total_limit
    'profile_regular_limit
    'transaction_total_limit
    'transaction_regular_limit,
    gas_limits_parameters(
        'block_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_total_limit,
        'transaction_regular_limit,
    ).
    GasLimitsFields(
        'block_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_total_limit,
        'transaction_regular_limit,
    )
}

/*! Couples the profile-selected caps to one concrete block header. This is
    evaluated once while the stateless input is installed. */
val gas_limits_for : forall 'fork 'target 'maximum 'denominator 'code_limit 'initcode_limit 'profile_total_limit 'profile_regular_limit 'transaction_blob_limit 'refund_divisor 'block_limit,
                          protocol_profile_parameters(
                              'fork,
                              'target,
                              'maximum,
                              'denominator,
                              'code_limit,
                              'initcode_limit,
                              'profile_total_limit,
                              'profile_regular_limit,
                              'transaction_blob_limit,
                              'refund_divisor,
                          )
                       &  0
                       <= 'block_limit
                       &  'block_limit
                       <= block_gas_limit_bound. (
        ProtocolProfileFields(
            'fork,
            'target,
            'maximum,
            'denominator,
            'code_limit,
            'initcode_limit,
            'profile_total_limit,
            'profile_regular_limit,
            'transaction_blob_limit,
            'refund_divisor,
        ),
        int('block_limit),
    ) -> GasLimitsFields(
        'block_limit,
        'profile_total_limit,
        'profile_regular_limit,
        if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit,
        if (if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit) < 'profile_regular_limit
        then (if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit)
        else 'profile_regular_limit,
    )
function gas_limits_for(profile, block_limit) = {
    let transaction_total_limit =
        if block_limit < profile.transaction_total_gas_limit then block_limit else profile.transaction_total_gas_limit;
    let transaction_regular_limit =
        if transaction_total_limit < profile.transaction_regular_gas_limit
        then transaction_total_limit
        else profile.transaction_regular_gas_limit;
    struct {
        block_limit = block_limit,
        transaction_total_limit = transaction_total_limit,
        transaction_regular_limit = transaction_regular_limit,
        system_regular_limit = SYSTEM_CALL_GAS_LIMIT,
        system_state_limit = 0,
    }
}

/*! A static protocol profile and the concrete limits derived from the current
    header, indexed together. Sharing the profile indices with `GasLimits`
    prevents independently valid but mutually inconsistent values from being
    paired. */
type execution_profile_parameters(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
    'code_limit : Int,
    'initcode_limit : Int,
    'profile_total_limit : Int,
    'profile_regular_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
    'block_limit : Int,
    'transaction_total_limit : Int,
    'transaction_regular_limit : Int,
) -> Bool =
       protocol_profile_parameters(
           'fork,
           'target,
           'maximum,
           'denominator,
           'code_limit,
           'initcode_limit,
           'profile_total_limit,
           'profile_regular_limit,
           'transaction_blob_limit,
           'refund_divisor,
       )
    &  gas_limits_parameters(
           'block_limit,
           'profile_total_limit,
           'profile_regular_limit,
           'transaction_total_limit,
           'transaction_regular_limit,
       )

/*! The static protocol profile paired with the gas limits derived from it
    for the executing block, sharing the profile indices so the pair cannot
    disagree. */
struct ExecutionProfileFields(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
    'code_limit : Int,
    'initcode_limit : Int,
    'profile_total_limit : Int,
    'profile_regular_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
    'block_limit : Int,
    'transaction_total_limit : Int,
    'transaction_regular_limit : Int,
), execution_profile_parameters(
    'fork,
    'target,
    'maximum,
    'denominator,
    'code_limit,
    'initcode_limit,
    'profile_total_limit,
    'profile_regular_limit,
    'transaction_blob_limit,
    'refund_divisor,
    'block_limit,
    'transaction_total_limit,
    'transaction_regular_limit,
) = {
    protocol : ProtocolProfileFields(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_blob_limit,
        'refund_divisor,
    ),
    gas : GasLimitsFields(
        'block_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_total_limit,
        'transaction_regular_limit,
    ),
}

/*! An execution profile with its thirteen indices packed existentially. */
type ExecutionProfile = {
    'fork
    'target
    'maximum
    'denominator
    'code_limit
    'initcode_limit
    'profile_total_limit
    'profile_regular_limit
    'transaction_blob_limit
    'refund_divisor
    'block_limit
    'transaction_total_limit
    'transaction_regular_limit,
    execution_profile_parameters(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_blob_limit,
        'refund_divisor,
        'block_limit,
        'transaction_total_limit,
        'transaction_regular_limit,
    ).
    ExecutionProfileFields(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_blob_limit,
        'refund_divisor,
        'block_limit,
        'transaction_total_limit,
        'transaction_regular_limit,
    )
}

/*! Installs the static protocol policy and its header-derived limits as one
    correlated value. */
val execution_profile_for : forall 'fork 'target 'maximum 'denominator 'code_limit 'initcode_limit 'profile_total_limit 'profile_regular_limit 'transaction_blob_limit 'refund_divisor 'block_limit,
                                 protocol_profile_parameters(
                                     'fork,
                                     'target,
                                     'maximum,
                                     'denominator,
                                     'code_limit,
                                     'initcode_limit,
                                     'profile_total_limit,
                                     'profile_regular_limit,
                                     'transaction_blob_limit,
                                     'refund_divisor,
                                 )
                              &  0
                              <= 'block_limit
                              &  'block_limit
                              <= block_gas_limit_bound. (
        ProtocolProfileFields(
            'fork,
            'target,
            'maximum,
            'denominator,
            'code_limit,
            'initcode_limit,
            'profile_total_limit,
            'profile_regular_limit,
            'transaction_blob_limit,
            'refund_divisor,
        ),
        int('block_limit),
    ) -> ExecutionProfileFields(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'profile_total_limit,
        'profile_regular_limit,
        'transaction_blob_limit,
        'refund_divisor,
        'block_limit,
        if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit,
        if (if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit) < 'profile_regular_limit
        then (if 'block_limit < 'profile_total_limit then 'block_limit else 'profile_total_limit)
        else 'profile_regular_limit,
    )
function execution_profile_for(protocol, block_limit) =
    struct { protocol = protocol, gas = gas_limits_for(protocol, block_limit) }

/*! The Amsterdam protocol profile: the sole schema the guest admits. It also
    initializes the kernel before input decoding. */
let DEFAULT_PROTOCOL_PROFILE : ProtocolProfile = {
    let profile = struct {
            fork = Amsterdam,
            blob_schedule =
                blob_schedule(
                    sizeof(bpo2_blob_target_count),
                    sizeof(bpo2_blob_max_count),
                    sizeof(bpo2_blob_fee_update_fraction),
                ),
            excess_blob_gas_limit =
                compute_profile_excess_blob_gas_limit(
                    Amsterdam,
                    sizeof(bpo2_blob_target_count),
                    sizeof(bpo2_blob_max_count),
                    sizeof(bpo2_blob_fee_update_fraction),
                ),
            deployed_code_size_limit = sizeof(amsterdam_deployed_code_size_limit),
            initcode_size_limit = sizeof(amsterdam_initcode_size_limit),
            transaction_total_gas_limit = sizeof(ssz_uint_bound),
            transaction_regular_gas_limit = sizeof(eip7825_transaction_gas_limit),
            transaction_blob_limit = sizeof(cancun_blob_max_count),
            refund_divisor = sizeof(post_london_refund_divisor),
        } :
            ProtocolProfileFields(
                amsterdam_fork_value,
                bpo2_blob_target_count,
                bpo2_blob_max_count,
                bpo2_blob_fee_update_fraction,
                amsterdam_deployed_code_size_limit,
                amsterdam_initcode_size_limit,
                ssz_uint_bound,
                eip7825_transaction_gas_limit,
                cancun_blob_max_count,
                post_london_refund_divisor,
            );
    pack_protocol_profile(profile)
}
let DEFAULT_EXECUTION_PROFILE : ExecutionProfile = execution_profile_for(DEFAULT_PROTOCOL_PROFILE, 0)
