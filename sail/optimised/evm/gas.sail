/* Attribute-only optimized-C refinements for the hot memory-gas path. */

$[c_inline]
val memory_word_count_word : word -> word

$[c_inline]
val mem_cost : forall 'words, memory_cost_input('words). int('words) -> {'cost,
                                                                         memory_cost_relation('words, 'cost).
                                                                         int('cost)}

$[c_inline]
val memory_required_size : forall 'start 'size, 0 <= 'start & 'start < 2 ^ 256 & 0 <= 'size & 'size < 2 ^ 256. (
        int('start),
        int('size),
    ) -> memory_required_endpoint

$[c_inline]
val memory_access : forall 'start 'size, 0 <= 'start & 'start < 2 ^ 256 & 0 <= 'size & 'size < 2 ^ 256. (
        int('start),
        int('size),
    ) -> MemoryAccess

$[c_inline]
val memory_expansion_gas_cost : forall 'required, memory_expansion_endpoint('required). (
        EvmMemorySlice,
        int('required),
        gas,
    ) -> GasCharge

$[c_inline]
val expand_memory : (EvmMemorySlice, memory_length) -> EvmMemorySlice
