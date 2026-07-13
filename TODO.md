# TODO

## Re-add keccak memoization for secure trie keys

The account/storage host hooks are keyed by raw `(address[, slot])`, and
`secure_keccak` (`ffi/state_db.c`) recomputes `keccak(address)` /
`keccak(slot)` on every call. Because keccak is a pure function, repeated
touches of the same address or slot within a block re-hash needlessly.

Previously this was memoized with a preimage → hash cache (open-addressing
hashtable, FNV-1a over the preimage words). It was removed to keep the key
derivation path stateless (no shared mutable memo global between transaction
state and the native base cache, simplifying the `eest_*` module split).

**Action:** reintroduce a preimage → keccak memo for `secure_keccak`, placed so
both the overlay (`state_db.c`) and the extracted native base cache
(`eest_state.c`) can share it without a backwards module dependency. Measure the
SLOAD/SSTORE hot-path impact (native Mgas/s + guest `instret_block`) to confirm
it earns its keep.
