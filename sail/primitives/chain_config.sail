
/*md
# Chain configuration

The decoded `SszChainConfig`. Its activation point is validated while decoding,
so the resulting value only carries the chain id. The schema prefix selects the
protocol profile, so neither the fork identifier nor its blob schedule is
repeated inside the SSZ body. Pure data — no registers, no externs.
*/
/*! The five protocol-defined blob schedules. Keeping the fields related
    excludes cross-products of individually valid constants which are not a
    schedule used by any fork. */
type blob_schedule_parameters(
    'target : Int,
    'maximum : Int,
    'denominator : Int,
) -> Bool =
       ('target == 0 & 'maximum == 0 & 'denominator == 1)
    |  ('target == 3 & 'maximum == 6 & 'denominator == 3338477)
    |  ('target == 6 & 'maximum == 9 & 'denominator == 5007716)
    |  ('target == 10 & 'maximum == 15 & 'denominator == 8346193)
    |  ('target == 14 & 'maximum == 21 & 'denominator == 11684671)

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

type BlobSchedule = {
    'target 'maximum 'denominator,
    blob_schedule_parameters('target, 'maximum, 'denominator).
    BlobScheduleFields('target, 'maximum, 'denominator)
}

/*! Constructs one admitted schedule while retaining its three singleton
    indices. */
val blob_schedule : forall 'target 'maximum 'denominator,
    blob_schedule_parameters('target, 'maximum, 'denominator).
    (
        int('target),
        int('maximum),
        int('denominator),
    ) -> BlobScheduleFields('target, 'maximum, 'denominator)
function blob_schedule(target, maximum, denominator) =
    struct { target = target, max = maximum, base_fee_update_fraction = denominator }

/*! Projections unpack the schedule's existential indices at the function
    boundary. */
function blob_schedule_target_of(schedule : BlobSchedule) -> blob_schedule_target_count =
    schedule.target

function blob_schedule_maximum_of(schedule : BlobSchedule) -> blob_schedule_max_count =
    schedule.max

function blob_schedule_denominator_of(schedule : BlobSchedule) -> blob_schedule_fee_update_fraction =
    schedule.base_fee_update_fraction

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
    'transaction_gas_limit : Int,
    'transaction_execution_gas_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
) -> Bool =
       (
           'fork == 5
           & 'target == 0
           & 'maximum == 0
           & 'denominator == 1
           & 'code_limit == 24576
           & 'initcode_limit == 0
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 0
           & 'refund_divisor == 2
       )
    |  (
           (6 <= 'fork & 'fork <= 9)
           & 'target == 0
           & 'maximum == 0
           & 'denominator == 1
           & 'code_limit == 24576
           & 'initcode_limit == 0
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 0
           & 'refund_divisor == 5
       )
    |  (
           'fork == 10
           & 'target == 0
           & 'maximum == 0
           & 'denominator == 1
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 0
           & 'refund_divisor == 5
       )
    |  (
           'fork == 11
           & 'target == 3
           & 'maximum == 6
           & 'denominator == 3338477
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 6
           & 'refund_divisor == 5
       )
    |  (
           'fork == 12
           & 'target == 6
           & 'maximum == 9
           & 'denominator == 5007716
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 9
           & 'refund_divisor == 5
       )
    |  (
           'fork == 13
           & 'target == 6
           & 'maximum == 9
           & 'denominator == 5007716
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 16777216
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 6
           & 'refund_divisor == 5
       )
    |  (
           'fork == 14
           & 'target == 10
           & 'maximum == 15
           & 'denominator == 8346193
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 16777216
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 6
           & 'refund_divisor == 5
       )
    |  (
           'fork == 15
           & 'target == 14
           & 'maximum == 21
           & 'denominator == 11684671
           & 'code_limit == 24576
           & 'initcode_limit == 49152
           & 'transaction_gas_limit == 16777216
           & 'transaction_execution_gas_limit == 0
           & 'transaction_blob_limit == 6
           & 'refund_divisor == 5
       )
    |  (
           'fork == 16
           & 'target == 14
           & 'maximum == 21
           & 'denominator == 11684671
           & 'code_limit == 65536
           & 'initcode_limit == 131072
           & 'transaction_gas_limit == 0
           & 'transaction_execution_gas_limit == 16777216
           & 'transaction_blob_limit == 6
           & 'refund_divisor == 5
       )

/*! Execution rules and computation bounds selected together by one schema
    fork. The singleton indices retain the exact selected parameter tuple. */
struct ProtocolProfileFields(
    'fork : Int,
    'target : Int,
    'maximum : Int,
    'denominator : Int,
    'code_limit : Int,
    'initcode_limit : Int,
    'transaction_gas_limit : Int,
    'transaction_execution_gas_limit : Int,
    'transaction_blob_limit : Int,
    'refund_divisor : Int,
), protocol_profile_parameters(
    'fork,
    'target,
    'maximum,
    'denominator,
    'code_limit,
    'initcode_limit,
    'transaction_gas_limit,
    'transaction_execution_gas_limit,
    'transaction_blob_limit,
    'refund_divisor,
) = {
    fork : int('fork),
    blob_schedule : BlobScheduleFields('target, 'maximum, 'denominator),
    deployed_code_size_limit : int('code_limit),
    initcode_size_limit : int('initcode_limit),
    transaction_gas_limit : int('transaction_gas_limit),
    transaction_execution_gas_limit : int('transaction_execution_gas_limit),
    transaction_blob_limit : int('transaction_blob_limit),
    refund_divisor : int('refund_divisor),
}

type ProtocolProfile = {
    'fork
    'target
    'maximum
    'denominator
    'code_limit
    'initcode_limit
    'transaction_gas_limit
    'transaction_execution_gas_limit
    'transaction_blob_limit
    'refund_divisor,
    protocol_profile_parameters(
        'fork,
        'target,
        'maximum,
        'denominator,
        'code_limit,
        'initcode_limit,
        'transaction_gas_limit,
        'transaction_execution_gas_limit,
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
        'transaction_gas_limit,
        'transaction_execution_gas_limit,
        'transaction_blob_limit,
        'refund_divisor,
    )
}

/*! Projections unpack the profile's existential indices at the function
    boundary while preserving the relationships between its parameters. */
function protocol_profile_fork(profile : ProtocolProfile) -> Fork =
    profile.fork

function protocol_profile_blob_schedule(profile : ProtocolProfile) -> BlobSchedule =
    profile.blob_schedule

function protocol_profile_deployed_code_size_limit(
    profile : ProtocolProfile
) -> protocol_deployed_code_size_limit =
    profile.deployed_code_size_limit

function protocol_profile_initcode_size_limit(
    profile : ProtocolProfile
) -> protocol_initcode_size_limit =
    profile.initcode_size_limit

function protocol_profile_transaction_gas_limit(
    profile : ProtocolProfile
) -> protocol_transaction_gas_limit =
    profile.transaction_gas_limit

function protocol_profile_transaction_execution_gas_limit(
    profile : ProtocolProfile
) -> protocol_transaction_execution_gas_limit =
    profile.transaction_execution_gas_limit

function protocol_profile_transaction_blob_limit(
    profile : ProtocolProfile
) -> transaction_blob_limit =
    profile.transaction_blob_limit

function protocol_profile_refund_divisor(
    profile : ProtocolProfile
) -> transaction_refund_divisor =
    profile.refund_divisor

/*! The schema's stable fork byte and complete protocol profile form one
    declarative Sail mapping. The mapping clauses are the sole table of
    admitted schema/profile combinations. */
mapping schema_protocol_profile : byte <-> ProtocolProfile = {
    forwards 0x0a => (struct {
        fork = Berlin,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 0,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 2,
    } : ProtocolProfileFields(5, 0, 0, 1, 24576, 0, 0, 0, 0, 2)),
    forwards 0x0b => (struct {
        fork = London,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 0,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 5,
    } : ProtocolProfileFields(6, 0, 0, 1, 24576, 0, 0, 0, 0, 5)),
    forwards 0x0c => (struct {
        fork = ArrowGlacier,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 0,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 5,
    } : ProtocolProfileFields(7, 0, 0, 1, 24576, 0, 0, 0, 0, 5)),
    forwards 0x0d => (struct {
        fork = GrayGlacier,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 0,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 5,
    } : ProtocolProfileFields(8, 0, 0, 1, 24576, 0, 0, 0, 0, 5)),
    forwards 0x0e => (struct {
        fork = Paris,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 0,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 5,
    } : ProtocolProfileFields(9, 0, 0, 1, 24576, 0, 0, 0, 0, 5)),
    forwards 0x0f => (struct {
        fork = Shanghai,
        blob_schedule = struct { target = 0, max = 0, base_fee_update_fraction = 1 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 0,
        refund_divisor = 5,
    } : ProtocolProfileFields(10, 0, 0, 1, 24576, 49152, 0, 0, 0, 5)),
    forwards 0x10 => (struct {
        fork = Cancun,
        blob_schedule = struct { target = 3, max = 6, base_fee_update_fraction = 3338477 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 6,
        refund_divisor = 5,
    } : ProtocolProfileFields(11, 3, 6, 3338477, 24576, 49152, 0, 0, 6, 5)),
    forwards 0x11 => (struct {
        fork = Prague,
        blob_schedule = struct { target = 6, max = 9, base_fee_update_fraction = 5007716 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 9,
        refund_divisor = 5,
    } : ProtocolProfileFields(12, 6, 9, 5007716, 24576, 49152, 0, 0, 9, 5)),
    forwards 0x12 => (struct {
        fork = Osaka,
        blob_schedule = struct { target = 6, max = 9, base_fee_update_fraction = 5007716 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 16777216,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 6,
        refund_divisor = 5,
    } : ProtocolProfileFields(13, 6, 9, 5007716, 24576, 49152, 16777216, 0, 6, 5)),
    forwards 0x13 => (struct {
        fork = BPO1,
        blob_schedule = struct { target = 10, max = 15, base_fee_update_fraction = 8346193 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 16777216,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 6,
        refund_divisor = 5,
    } : ProtocolProfileFields(14, 10, 15, 8346193, 24576, 49152, 16777216, 0, 6, 5)),
    forwards 0x14 => (struct {
        fork = BPO2,
        blob_schedule = struct { target = 14, max = 21, base_fee_update_fraction = 11684671 },
        deployed_code_size_limit = 24576,
        initcode_size_limit = 49152,
        transaction_gas_limit = 16777216,
        transaction_execution_gas_limit = 0,
        transaction_blob_limit = 6,
        refund_divisor = 5,
    } : ProtocolProfileFields(15, 14, 21, 11684671, 24576, 49152, 16777216, 0, 6, 5)),
    forwards 0x15 => (struct {
        fork = Amsterdam,
        blob_schedule = struct { target = 14, max = 21, base_fee_update_fraction = 11684671 },
        deployed_code_size_limit = 65536,
        initcode_size_limit = 131072,
        transaction_gas_limit = 0,
        transaction_execution_gas_limit = 16777216,
        transaction_blob_limit = 6,
        refund_divisor = 5,
    } : ProtocolProfileFields(16, 14, 21, 11684671, 65536, 131072, 0, 16777216, 6, 5)),
}

/*! The Amsterdam mapping entry initializes the kernel before input decoding. */
let DEFAULT_PROTOCOL_PROFILE : ProtocolProfile = schema_protocol_profile(0x15)

/*! The decoded chain configuration for the executing payload. */
struct ChainConfig = {
    chain_id : chain_identifier,
}
