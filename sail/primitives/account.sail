/*md
# Accounts and storage

The account tuple and its sentinels, and the per-layer account/storage
entry types that cross the host interface (overlay rows, merge
enumeration). Pure
data — no registers, no externs.
*/

/*! Account state (Yellow Paper §4.1, the account tuple σ[a]).
    `storage_root` is the account's *pre-state* storage root — the witness
    anchor that stateless storage reads walk on a cached-state miss. It is
    set when the account is materialized from the witness and is not kept
    live during execution (storage mutates in the overlay maps; the
    post-state root is computed on demand). Code bytes are not held here —
    the account carries only `code_hash`, a content address into the code
    store; the code hash is itself observable state (`EXTCODEHASH`,
    EIP-1052) and the binding a stateless witness checks code against. */
struct AccountInfo = {
    nonce        : u64,   /* sigma[a]_n: nonce (u64: EIP-2681 caps it at 2^64-1)   */
    balance      : word,  /* sigma[a]_b: balance in wei                            */
    code_hash    : hash,  /* sigma[a]_c: keccak256(code); KECCAK_EMPTY if codeless */
    storage_root : hash,  /* sigma[a]_s: PRE-STATE storage root, the witness anchor */
}

/*! An account plus its lifecycle flags: existence, EIP-161 storage
    clearing, same-transaction creation (EIP-6780), and selfdestruction. */
struct Account = {
    info           : AccountInfo,
    exists         : bool,
    storage_cleared: bool,
    created        : bool,
    selfdestructed : bool,
}

/*! A storage slot's current and original (transaction-start) values —
    the pair EIP-2200/EIP-3529 gas and refund rules compare. */
struct StorageValue = { curr : word, orig : word }

/*! A fully qualified storage key: account address and 256-bit slot. */
struct StorageKey = { addr : address, slot : word }

/*! An overlay storage row: the form in which storage entries are
    enumerated at transaction-end merge. */
struct StorageEntry = { key : StorageKey, value : StorageValue }

/*! An account's current and original (transaction-start) states. */
struct AcctValue = { curr : Account, orig : Account }

/*! An overlay account row: the form in which account entries are
    enumerated at transaction-end merge. */
struct AcctEntry = { addr : address, value : AcctValue }

/*! The empty account tuple: zero nonce and balance, `KECCAK_EMPTY` code
    hash, empty-trie storage root (YP §4.1). */
let EMPTY_ACCOUNT_INFO : AccountInfo = struct {
        nonce = sail_zeros(64),
        balance = ZERO_WORD,
        code_hash = KECCAK_EMPTY,
        storage_root = EMPTY_TRIE_ROOT,
    }

/*! The non-existent account sentinel (EIP-161 "empty"). */
let EMPTY_ACCOUNT : Account = struct {
        info = EMPTY_ACCOUNT_INFO,
        exists = false,
        storage_cleared = true,
        created = false,
        selfdestructed = false,
    }

/*! Wraps a witnessed account tuple as an existing, unmodified account. */
function account_from_info(info : AccountInfo) -> Account =
    struct { info = info, exists = true, storage_cleared = false, created = false, selfdestructed = false }
