/* ======================================================================== */
/* PRIMITIVES: ACCOUNT + STORAGE TYPES                                      */
/*                                                                          */
/* The account tuple and its sentinels, and the per-layer account/storage   */
/* entry types that cross the C boundary (overlay rows, merge drains).      */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

/* Account state (Yellow Paper section 4.1, the account tuple sigma[a]).
 * storage_root is the account's PRE-STATE storage root -- the witness anchor
 * that stateless storage reads walk on a working-set miss. It is set when the
 * account is materialized from the witness and is NOT kept live during execution
 * (storage mutates in the overlay maps; the post-state root is computed on demand
 * in state_root.sail). Code BYTES are not held here -- the account carries only
 * codeHash, a content address into the code store; codeHash is itself observable
 * state (EXTCODEHASH, EIP-1052) and the binding a stateless witness checks code
 * against. */
struct Account = {
    nonce        : u64,   /* sigma[a]_n: nonce (u64: EIP-2681 caps it at 2^64-1)   */
    balance      : word,  /* sigma[a]_b: balance in wei                            */
    code_hash     : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
    storage_root : hash,  /* sigma[a]_s: PRE-STATE storage root, the witness anchor */
}

/* keccak256 of the empty string: the codeHash of every codeless account */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* keccak256(rlp("")) -- the root of an empty Merkle-Patricia trie: the storage
 * root of every account with no storage (EMPTY_ACCOUNT, freshly created). */
let EMPTY_TRIE_ROOT : hash = 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421

struct StorageValue = { curr : word, orig : word }

struct StorageKey = { addr : address, slot : word }

struct StorageEntry = { key : StorageKey, value : StorageValue }

struct AcctValue = { curr : Account, orig : Account }

struct AcctEntry = { addr : address, value : AcctValue }

let EMPTY_ACCOUNT : Account = struct {
        nonce = sail_zeros(64),
        balance = ZERO_WORD,
        code_hash = KECCAK_EMPTY,
        storage_root = EMPTY_TRIE_ROOT,
    }
