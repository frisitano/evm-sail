/* Optimized C overrides for `sail/host/accelerators.sail`. */

/*
 * Optional cycle-scope overlay for optimized C guests.
 *
 * The executable specification contains no performance instrumentation.
 * Profile builds apply this overlay after the ordinary optimized modules,
 * replacing only orchestration functions at useful measurement boundaries.
 * Scope numbers remain stable because benchmark profiles persist them outside
 * the model.
 */

type cycle_scope_id = range(0, 255)

val cycle_scope_start = impure { c: "cycle_scope_start" } : cycle_scope_id -> unit
val cycle_scope_end = impure { c: "cycle_scope_end" } : cycle_scope_id -> unit
