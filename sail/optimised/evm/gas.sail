/* Optimized C overrides for `sail/evm/gas.sail`. */

/*! Saturates only the optimized representation. Exact canonical and proof
    extractions retain the mathematical `start + size` endpoint. */
function memory_required_size(start, size) =
    if size == 0 then {
        0
    } else if start <= sizeof(memory_region_bound) then {
        let (bounded_start as 'bounded_start) :
                {'bounded_start,
                 'bounded_start == 'start
                 & 0 <= 'bounded_start
                 & 'bounded_start <= memory_region_bound.
                 int('bounded_start)} = start;
        if size <= sizeof(memory_region_bound) - bounded_start then {
            let (bounded_size as 'bounded_size) :
                    {'bounded_size,
                     'bounded_size == 'size
                     & 0 < 'bounded_size
                     & 'bounded_size <= memory_region_bound - 'bounded_start.
                     int('bounded_size)} = size;
            bounded_start + bounded_size
        } else {
            sizeof(memory_region_bound + 1)
        }
    } else {
        sizeof(memory_region_bound + 1)
    }

/*! Computes the exact incremental cost for materializable endpoints and uses
    the proven-unaffordable charge sentinel for every saturated endpoint. */
function memory_expansion_cost(required_size) =
    if required_size <= sizeof(memory_region_bound) then {
        let materialized_size : memory_length = required_size;
        let new_words = memory_word_count(materialized_size);
        let old_words = memory_word_count(evm_memory_high_water());
        if new_words <= old_words then {
            0
        } else {
            let old_cost = mem_cost(old_words);
            let new_cost = mem_cost(new_words);
            if old_cost <= new_cost then { new_cost - old_cost } else { 0 }
        }
    } else {
        2 ^ 64
    }
