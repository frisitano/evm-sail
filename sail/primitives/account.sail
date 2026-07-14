/* ======================================================================== */
/* PRIMITIVES: ACCOUNT + STORAGE TYPES                                      */
/*                                                                          */
/* The account tuple and its sentinels, and the per-layer account/storage   */
/* entry types that cross the C boundary (overlay rows, merge drains).      */
/* Pure data -- no registers, no externs.                                   */
/* ======================================================================== */

/* Account state (Yellow Paper section 4.1, the account tuple sigma[a]).
 * storage_root is the account's PRE-STATE storage root -- the witness anchor
 * that stateless storage reads walk on a cached-state miss. It is set when the
 * account is materialized from the witness and is NOT kept live during execution
 * (storage mutates in the overlay maps; the post-state root is computed on demand
 * in state_root.sail). Code BYTES are not held here -- the account carries only
 * codeHash, a content address into the code store; codeHash is itself observable
 * state (EXTCODEHASH, EIP-1052) and the binding a stateless witness checks code
 * against. */
struct AccountInfo = {
    nonce        : u64,   /* sigma[a]_n: nonce (u64: EIP-2681 caps it at 2^64-1)   */
    balance      : word,  /* sigma[a]_b: balance in wei                            */
    code_hash    : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
    storage_root : hash,  /* sigma[a]_s: PRE-STATE storage root, the witness anchor */
}

struct Account = {
    info           : AccountInfo,
    exists         : bool,
    storage_cleared: bool,
    created        : bool,
    selfdestructed : bool,
}

struct StorageValue = { curr : word, orig : word }

struct StorageKey = { addr : address, slot : word }

struct StorageEntry = { key : StorageKey, value : StorageValue }

struct AcctValue = { curr : Account, orig : Account }

struct AcctEntry = { addr : address, value : AcctValue }

let EMPTY_ACCOUNT_INFO : AccountInfo = struct {
        nonce = sail_zeros(64),
        balance = ZERO_WORD,
        code_hash = KECCAK_EMPTY,
        storage_root = EMPTY_TRIE_ROOT,
    }

let EMPTY_ACCOUNT : Account = struct {
        info = EMPTY_ACCOUNT_INFO,
        exists = false,
        storage_cleared = true,
        created = false,
        selfdestructed = false,
    }

function account_from_info(info : AccountInfo) -> Account =
    struct { info = info, exists = true, storage_cleared = false, created = false, selfdestructed = false }
