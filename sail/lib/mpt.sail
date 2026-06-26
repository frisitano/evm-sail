/* ======================================================================== */
/* MERKLE-PATRICIA TRIE                                                     */
/*                                                                          */
/* The authenticated key/value structure underlying Ethereum world state    */
/* (Yellow Paper Appendix D), with hex-prefix path encoding (Appendix C).   */
/* This module specifies one trie in three layers.                          */
/*                                                                          */
/* (1) Root builder: the map TRIE(.) from a set of key/value pairs to a     */
/* 32-byte root. Sorted (nibble-key, value) leaves fold bottom-up into leaf */
/* / extension / branch nodes; a node is embedded in its parent when its    */
/* RLP encoding is under 32 bytes, otherwise referenced by keccak-256.      */
/*                                                                          */
/* (2) World-state trie: the state root over accounts, keyed by             */
/* keccak256(address) with value RLP([nonce, balance, storageRoot,          */
/* codeHash]) over per-account storage tries. This is the commitment a      */
/* block's stateRoot must equal.                                            */
/*                                                                          */
/* (3) Witness reads: stateless verification -- re-root a partial trie from */
/* a node witness to authenticate it against a committed root, and resolve  */
/* account/storage values by Merkle proof.                                  */
/* ======================================================================== */

/* ======================================================================== */
/* (1) trie root builder                                                    */
/*                                                                          */
/* Computes the trie root of a set of key/value pairs (Yellow Paper         */
/* Appendix D, the structural composition c). Leaves are supplied in        */
/* ascending nibble-key order and folded bottom-up through an explicit node */
/* stack with per-depth branch masks: a diverging key collapses the stack,  */
/* emitting the leaf, extension, and branch nodes along the closed path.    */
/* Each node is RLP-encoded (with hex-prefix paths, Appendix C) and         */
/* referenced by its parent inline when under 32 bytes, else by 0xa0 ++     */
/* keccak256(node).                                                         */
/*                                                                          */
/* A node known only by hash -- a blinded subtree on a witness frontier --  */
/* is admitted directly (hb_add_branch), so a PARTIAL trie can be re-rooted */
/* and checked against a committed root in one pass. This is the basis of   */
/* the stateless pre-state verification in layer (3).                       */
/* ======================================================================== */

/* ---- small generic list helpers (take/drop n from the front) ----------- */
/* ---- shared trie primitives (formerly lib/mpt.sail) -------------------- */

/* (key-nibbles, value) pair: the unit a trie root is built from. */
struct TriePair = { tp_key : list(int), tp_val : list(byte) }

/* 32-byte key -> 64 nibbles, high nibble first (single pass + reverse). */
function to_nibbles(bs : list(byte)) -> list(int) = {
    var out : list(int) = [||];
    var rest : list(byte) = bs;
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            b :: r => {
                let bb = unsigned(b);
                out = ediv_int(bb, 16) :: out;
                out = emod_int(bb, 16) :: out;
                rest = r
            },
            [||] => (),
        }
    };
    list_reverse(out)
}

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag (YP C.1). */
function hp_compact(nibs : list(int), is_leaf : bool) -> list(byte) = {
    let len = list_len(nibs);
    let odd = emod_int(len, 2) == 1;
    let flag : int = if is_leaf then { 2 } else { 0 };
    var out : list(byte) = [||];
    var rest : list(int) = nibs;
    if odd then {
        match rest {
            n0 :: t => {
                out = get_slice_int(8, (flag + 1) * 16 + n0, 0) :: out;
                rest = t
            },
            [||] => (),
        }
    } else {
        out = get_slice_int(8, flag * 16, 0) :: out
    };
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            a :: (b :: t) => {
                out = get_slice_int(8, a * 16 + b, 0) :: out;
                rest = t
            },
            _ => rest = [||],
        }
    };
    list_reverse(out)
}

/* reference to a child node (Yellow Paper Appendix D, the node cap function n):
 * the node's RLP inline when under 32 bytes, otherwise 0xa0 ++ keccak256(node). */
function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
    }

/* A trie leaf VALUE is the RLP encoding of the slot value (a string), e.g.
 * RLP(132) = 0x81 0x84, RLP(32-byte) = 0xa0 ++ 32 bytes. rlp_field_word reads
 * rf_content as a raw big-endian word, which is only correct AFTER the RLP
 * string header is stripped: decode the short-string header (0x80..0xb7 -> drop
 * the length byte; a single byte < 0x80 is the value itself). Without this, a
 * small value keeps its 0x81 prefix (e.g. -> 0x8184) and is double-encoded on
 * re-RLP; large 32-byte values happened to work only because the 0xa0 prefix
 * overflows out of the 256-bit word. */
function rlp_str_word(bs : list(byte)) -> word =
    match bs {
        [||] => sail_zeros(256),
        b0 :: rest => if unsigned(b0) < 128 then {
            rlp_field_word(struct { rf_is_list = false, rf_content = bs, rf_full = bs })
        } else {
            rlp_field_word(struct { rf_is_list = false, rf_content = rest, rf_full = rest })
        },
    }

function hb_ltake forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var out : list('a) = [||];
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            h :: t => {
                out = h :: out;
                rest = t
            },
            [||] => (),
        };
        i = i + 1
    };
    list_reverse(out)
}
function hb_ldrop forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            _ :: t => rest = t,
            [||] => (),
        };
        i = i + 1
    };
    rest
}

/* ---- bit / mask helpers (16-bit branch masks held as int) --------------- */
function hb_pow2(n : int) -> int = {
    var r : int = 1;
    var i : int = 0;
    while i < n do {
        r = r * 2;
        i = i + 1
    };
    r
}
function hb_bit_set(mask : int, i : int) -> bool = emod_int(ediv_int(mask, hb_pow2(i)), 2) == 1
function hb_with_bit(mask : int, i : int) -> int = if hb_bit_set(mask, i) then {
    mask
} else {
    mask + hb_pow2(i)
}
function hb_popcount16(mask : int) -> int = {
    var c : int = 0;
    foreach (i from 0 to 15) {
        if hb_bit_set(mask, i) then {
            c = c + 1
        }
    };
    c
}

/* mask-vector ops over list(int) (index 0 = depth 0) */
function hb_mlen(ms : list(int)) -> int = list_len(ms)
function hb_mget(ms : list(int), i : int) -> int = list_nth(ms, i, 0)
function hb_mset(ms : list(int), i : int, v : int) -> list(int) = {
    let l = if hb_mlen(ms) > (i + 1) then {
            hb_mlen(ms)
        } else {
            i + 1
        };
    var out : list(int) = [||];
    foreach (k from 0 to (l - 1)) {
        out = (if k == i then v else list_nth(ms, k, 0)) :: out
    };
    list_reverse(out)
}

/* longest common prefix length of two nibble lists */
function hb_cpl(a : list(int), b : list(int)) -> int = {
    var i : int = 0;
    var ra : list(int) = a;
    var rb : list(int) = b;
    var stop : bool = false;
    while not_bool(stop) do {
        match (ra, rb) {
            (x :: ta, y :: tb) => if x == y then {
                i = i + 1;
                ra = ta;
                rb = tb
            } else {
                stop = true
            },
            (_, _) => stop = true,
        }
    };
    i
}

/* 32 big-endian bytes -> bits(256) (local; avoids clash with mpt_witness) */
function hb_bytes_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(list_nth(c, k, 0x00), 256))
    };
    acc
}

/* ---- node child-refs (the values the stack holds) ----------------------- */
/* leaf:      child_ref( rlp[ hp_compact(key,leaf) , value ] )                 */
/* extension: child_ref( rlp[ hp_compact(key,ext)  , child-ref ] )            */
/* branch:    child_ref( rlp[ 16 child-refs-or-0x80 , 0x80 ] )                 */
/* word:      0xa0 ++ hash   (a blinded subtree / >=32B child, already a ref)  */
function hb_leaf_ref(key : list(int), value : list(byte)) -> list(byte) =
    child_ref(rlp_list(list_append(rlp_bytes(hp_compact(key, true)), rlp_bytes(value))))
function hb_ext_ref(key : list(int), childref : list(byte)) -> list(byte) =
    child_ref(rlp_list(list_append(rlp_bytes(hp_compact(key, false)), childref)))
function hb_word_ref(hash : list(byte)) -> list(byte) = list_append([|0xa0|], hash)

function hb_branch_payload(mask : int, children : list(list(byte))) -> list(byte) = {
    var payload : list(byte) = [||];
    var ch : list(list(byte)) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if hb_bit_set(mask, i) then {
            match ch {
                c :: t => {
                    payload = list_append(payload, c);
                    ch = t
                },
                [||] => (),
            }
        } else {
            payload = list_append(payload, [|0x80|])
        }
    };
    list_append(payload, [|0x80|]) /* branch value slot: empty */
}
function hb_branch_ref(mask : int, children : list(list(byte))) -> list(byte) =
    child_ref(rlp_list(hb_branch_payload(mask, children)))

/* ---- HashBuilder state (registers; single-threaded root computation) ---- */
register hb_key : list(int) = [||] /* current key nibbles            */
register hb_val : list(byte) = [||] /* current value (leaf) or hash    */
register hb_is_hash : bool = false /* value is a blinded branch hash  */
register hb_stack : list(list(byte)) = [||] /* stack of child-refs; head = top */
register hb_masks : list(int) = [||] /* state masks; index 0 = depth 0  */

function hb_reset() -> unit = {
    hb_key = [||];
    hb_val = [||];
    hb_is_hash = false;
    hb_stack = [||];
    hb_masks = [||]
}

/* build a branch node at depth `len` from the top popcount(mask) stack items */
function hb_push_branch(len : int) -> unit = {
    let mask = hb_mget(hb_masks, len);
    let k = hb_popcount16(mask);
    let children = list_reverse(hb_ltake(hb_stack, k)); /* head=highest nibble -> ascending */
    let r = hb_branch_ref(mask, children);
    hb_stack = r :: hb_ldrop(hb_stack, k)
}

/* the core collapse loop: fold the stack up to the common prefix with `succ` */
function hb_update(succ : list(int)) -> unit = {
    var build_extensions : bool = false;
    var current : list(int) = hb_key;
    var done : bool = false;
    while not_bool(done) do {
        let preceding_exists = hb_mlen(hb_masks) > 0;
        let preceding_len = if hb_mlen(hb_masks) == 0 then {
                0
            } else {
                hb_mlen(hb_masks) - 1
            };
        let common = hb_cpl(succ, current);
        let len = if preceding_len > common then { preceding_len } else { common };
        if len >= list_len(current) then {
            /* inconsistent harvested leaves (a deficient witness feeding out-of-
               order / duplicate keys): graceful invalid -- stop building rather
               than abort. */
            k_witness_bad = true;
            done = true
        } else {
            let extra = list_nth(current, len, 0);
            hb_masks = hb_mset(hb_masks, len, hb_with_bit(hb_mget(hb_masks, len), extra));
            let len_from = if (list_len(succ) > 0) | preceding_exists then {
                    len + 1
                } else {
                    len
                };
            let short_key = hb_ldrop(current, len_from);
            if not_bool(build_extensions) then {
                if hb_is_hash then {
                    hb_stack = hb_word_ref(hb_val) :: hb_stack;
                    build_extensions = true
                } else {
                    hb_stack = hb_leaf_ref(short_key, hb_val) :: hb_stack
                }
            };
            if build_extensions & (list_len(short_key) > 0) then {
                match hb_stack {
                    last :: rest => hb_stack = hb_ext_ref(short_key, last) :: rest,
                    [||] => (),
                }
            };
            if (preceding_len <= common) & (list_len(succ) > 0) then {
                done = true
            } else {
                if (list_len(succ) > 0) | preceding_exists then {
                    hb_push_branch(len)
                };
                hb_masks = hb_ltake(hb_masks, len);
                if preceding_len == 0 then {
                    done = true
                } else {
                    current = hb_ltake(current, preceding_len);
                    var popping : bool = true;
                    while popping do {
                        let l = hb_mlen(hb_masks);
                        if l > 0 & hb_mget(hb_masks, l - 1) == 0 then {
                            hb_masks = hb_ltake(hb_masks, l - 1)
                        } else {
                            popping = false
                        }
                    };
                    build_extensions = true
                }
            }
        }
    }
}

function hb_add_leaf(key : list(int), value : list(byte)) -> unit = {
    if list_len(hb_key) > 0 then {
        hb_update(key)
    };
    hb_key = key;
    hb_val = value;
    hb_is_hash = false
}
function hb_add_branch(key : list(int), hash : list(byte)) -> unit = {
    if list_len(hb_key) > 0 then {
        hb_update(key)
    } else {
        if list_len(key) == 0 then {
            hb_stack = hb_word_ref(hash) :: hb_stack
        }
    };
    hb_key = key;
    hb_val = hash;
    hb_is_hash = true
}

function hb_current_root() -> bits(256) =
    match hb_stack {
        [||]     => keccak256([|0x80|]), /* empty trie: keccak(rlp("")) */
        top :: _ => if list_len(top) == 33 then {
            hb_bytes_be256(hb_ldrop(top, 1))
        } else {
            keccak256(top)
        },
    }
function hb_root() -> bits(256) = {
    if list_len(hb_key) > 0 then {
        hb_update([||]);
        hb_key = [||];
        hb_val = [||]
    };
    hb_current_root()
}

/* feed (already key-sorted) pairs through hb_add_leaf and return the root.
 * The sole MPT-root driver: storage roots and the state root both use it. */
function hb_root_of(pairs : list(TriePair)) -> bits(256) = {
    hb_reset();
    var ps : list(TriePair) = pairs;
    while match ps {
        [||] => false,
        _ => true,
    } do {
        match ps {
            p :: r => {
                hb_add_leaf(p.tp_key, p.tp_val);
                ps = r
            },
            [||] => (),
        }
    };
    hb_root()
}

/* ======================================================================== */
/* (2) Ethereum world-state trie                                            */
/*                                                                          */
/* The state root (Yellow Paper section 4.1): the secure-trie root over all */
/* accounts. The trie is 'secure' -- keys are hashed: key =                 */
/* keccak256(address), value = RLP([nonce, balance, storageRoot,            */
/* codeHash]). storageRoot is the root of the account's storage trie, the   */
/* secure trie over keccak256(slot) -> RLP(value) with zero-valued slots    */
/* absent. Accounts that are empty per EIP-161 (zero nonce, zero balance,   */
/* no code) are excluded.                                                   */
/*                                                                          */
/* compute_state_root_hb over the post-execution world equals the block's   */
/* stateRoot exactly when execution is correct -- it authenticates the      */
/* result.                                                                  */
/* ======================================================================== */

/* Reused account-RLP payload accumulator. account_rlp is invoked once per account
 * when authenticating the post-state root (compute_state_root); a block-level
 * root or the proving path runs it over the whole account set, so it builds the
 * payload into this single register (O(1) prepend, reversed once) instead of
 * allocating fresh intermediate lists per account. Not reentrant: cleared at the
 * start of every account_rlp call, and account_rlp calls nothing that touches it. */
register rlp_scratch : list(byte) = [||]
function rlp_push(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                rlp_scratch = b :: rlp_scratch;
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* rlp([nonce, balance, storageRoot, codeHash]) */
function account_rlp(nonce : int, balance : word, sroot : bits(256), chash : bits(256)) -> list(byte) = {
    rlp_scratch = [||];
    rlp_push(rlp_int(nonce));
    rlp_push(rlp_int(unsigned(balance)));
    rlp_push(rlp_word(sroot));
    rlp_push(rlp_word(chash));
    rlp_list(list_reverse(rlp_scratch)) /* prepend builds newest-first; reverse -> field order */
}

/* ---- post-state root via the stack HashBuilder ------------------------- */
/* lexicographic nibble-key order (HashBuilder requires strictly ascending keys) */
function nib_lt(a : list(int), b : list(int)) -> bool = {
    var ra : list(int) = a;
    var rb : list(int) = b;
    var res : int = 0; /* 0 undecided, 1 a<b, 2 a>=b */
    while res == 0 do {
        match (ra, rb) {
            ([||], [||]) => res = 2,
            ([||], _) => res = 1,
            (_, [||]) => res = 2,
            (x :: ta, y :: tb) => if x < y then {
                res = 1
            } else {
                if x > y then {
                    res = 2
                } else {
                    ra = ta;
                    rb = tb
                }
            },
        }
    };
    res == 1
}
function insert_sorted(p : TriePair, ps : list(TriePair)) -> list(TriePair) =
    match ps {
        [||]   => [|p|],
        q :: r => if nib_lt(p.tp_key, q.tp_key) then {
            p :: ps
        } else {
            q :: insert_sorted(p, r)
        },
    }
function sort_pairs(ps : list(TriePair)) -> list(TriePair) =
    match ps {
        [||] => [||],
        q :: r => insert_sorted(q, sort_pairs(r)),
    }

/* C-backed witness storage-slot store (ffi/host_nodedb.c):
 * (keccak(addr), keccak(slot)) -> 256-bit value. The harvested, authenticated
 * pre-state storage. Point lookup (slotdb_sel + slotdb_selval) serves stateless
 * reads; iteration (slotdb_count + slotdb_at + slotdb_at_*) feeds the post-state
 * root. Each 256-bit key half crosses as four big-endian words (w3 most sig). */
val slotdb_reset = impure { _: "slotdb_reset" } : unit -> unit
val slotdb_insert = impure { _: "slotdb_insert" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val slotdb_sel = impure { _: "slotdb_sel" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
val slotdb_selval = impure { _: "slotdb_selval" } : bits(64) -> bits(64)
val slotdb_count = impure { _: "slotdb_count" } : unit -> bits(64)
val slotdb_at = impure { _: "slotdb_at" } : bits(64) -> unit
val slotdb_at_acct = impure { _: "slotdb_at_acct" } : bits(64) -> bits(64)
val slotdb_at_slot = impure { _: "slotdb_at_slot" } : bits(64) -> bits(64)
val slotdb_at_val = impure { _: "slotdb_at_val" } : bits(64) -> bits(64)

/* harvest context: while re-rooting an account's storage trie, witness_feed's
 * leaf sink switches from the account leaf store to slotdb, keyed by hv_acct. */
register hv_storage : bool = false
register hv_acct : bits(256) = sail_zeros(256)

/* insert a harvested (acct, slot) -> value into the slot store */
function slot_insert(acct : bits(256), slot : bits(256), v : word) -> unit =
    slotdb_insert(
        acct[255 .. 192],
        acct[191 .. 128],
        acct[127 .. 64],
        acct[63 .. 0],
        slot[255 .. 192],
        slot[191 .. 128],
        slot[127 .. 64],
        slot[63 .. 0],
        v[255 .. 192],
        v[191 .. 128],
        v[127 .. 64],
        v[63 .. 0],
    )

/* the cached slotdb_sel value as a 256-bit word */
function slot_selword() -> word =
    append(
        slotdb_selval(get_slice_int(64, 3, 0)),
        append(
            slotdb_selval(get_slice_int(64, 2, 0)),
            append(slotdb_selval(get_slice_int(64, 1, 0)), slotdb_selval(get_slice_int(64, 0, 0))),
        ),
    )

/* 256-bit acct key / slot key / value of the currently-selected slotdb row */
function slot_at_slot() -> bits(256) =
    append(
        slotdb_at_slot(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_slot(get_slice_int(64, 2, 0)),
            append(slotdb_at_slot(get_slice_int(64, 1, 0)), slotdb_at_slot(get_slice_int(64, 0, 0))),
        ),
    )
function slot_at_val() -> word =
    append(
        slotdb_at_val(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_val(get_slice_int(64, 2, 0)),
            append(slotdb_at_val(get_slice_int(64, 1, 0)), slotdb_at_val(get_slice_int(64, 0, 0))),
        ),
    )
function slot_at_acct() -> bits(256) =
    append(
        slotdb_at_acct(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_acct(get_slice_int(64, 2, 0)),
            append(slotdb_at_acct(get_slice_int(64, 1, 0)), slotdb_at_acct(get_slice_int(64, 0, 0))),
        ),
    )

/* membership test for a slot key in the written-slot list (dedup helper) */
function h256_in(xs : list(bits(256)), h : bits(256)) -> bool =
    match xs {
        [||]   => false,
        y :: r => if y == h then {
            true
        } else {
            h256_in(r, h)
        },
    }

/* secure-trie pairs for one account's post-state storage: the map writes
 * (overlay, authoritative) merged over the harvested witness pre-state slots
 * (slotdb) not overwritten by a write. A slot written to 0 is recorded as a
 * written key (excluding the harvested pre-value) but contributes no pair, so a
 * cleared slot is absent from the trie. Cold path (block-end authentication). */
function storage_pairs(a : address) -> list(TriePair) = {
    let ak = pre_keccak_addr(a);
    var written : list(bits(256)) = [||]; /* keccak(slot) of every map write */
    var out : list(TriePair) = [||];
    let n = storage_count();
    foreach (j from 0 to (n - 1)) {
        let (ea, es, ev) = storage_entry(j);
        if ea == a then {
            let sk = pre_keccak_slot(es);
            written = sk :: written;
            if not_bool(word_is_zero(ev)) then {
                out = struct { tp_key = to_nibbles(word_to_bytes32(sk)), tp_val = rlp_int(unsigned(ev)) } :: out
            }
        }
    };
    let m = unsigned(slotdb_count());
    foreach (j from 0 to (m - 1)) {
        slotdb_at(get_slice_int(64, j, 0));
        if slot_at_acct() == ak then {
            let sk = slot_at_slot();
            let sv = slot_at_val();
            if not_bool(word_is_zero(sv)) & not_bool(h256_in(written, sk)) then {
                out = struct { tp_key = to_nibbles(word_to_bytes32(sk)), tp_val = rlp_int(unsigned(sv)) } :: out
            }
        }
    };
    out
}

/* post-state storage root of `a`. Incremental: the present + written leaves
 * (storage_pairs) are merged with the account's BLINDED pre-state subtrees and
 * folded by the stack builder, so untouched subtrees are reused by hash rather
 * than re-enumerated (a flat enumeration would silently drop them -> wrong root
 * for a minimal/partial witness). Defined after the witness reader (it walks the
 * storage trie via node_db_lookup); forward-declared here for state_pairs. */
val storage_root : address -> bits(256)

function acc_is_empty(acc : Account) -> bool =
    (acc.acc_codehash == KECCAK_EMPTY) & acc.acc_nonce == 0 & word_is_zero(acc.acc_balance)

/* 256-bit account field (balance / code_hash) of the current acctmap_at row */
function am_at_bal() -> word =
    append(
        acctmap_at_bal(get_slice_int(64, 3, 0)),
        append(
            acctmap_at_bal(get_slice_int(64, 2, 0)),
            append(acctmap_at_bal(get_slice_int(64, 1, 0)), acctmap_at_bal(get_slice_int(64, 0, 0))),
        ),
    )
function am_at_chash() -> word =
    append(
        acctmap_at_chash(get_slice_int(64, 3, 0)),
        append(
            acctmap_at_chash(get_slice_int(64, 2, 0)),
            append(acctmap_at_chash(get_slice_int(64, 1, 0)), acctmap_at_chash(get_slice_int(64, 0, 0))),
        ),
    )

/* address of the current acctmap_at row (key words a2,a1,a0) */
function am_at_addr() -> address =
    word_to_address(
        sail_zero_extend(
            append(
                acctmap_at_key(get_slice_int(64, 0, 0)),
                append(acctmap_at_key(get_slice_int(64, 1, 0)), acctmap_at_key(get_slice_int(64, 2, 0))),
            ),
            256,
        ),
    )

/* secure-trie pairs for all non-empty accounts, by iterating the acctmap. Reads
 * each row's fields into locals before computing its storage_root (which iterates
 * the storage maps -- disjoint from the acctmap iteration cache). */
function state_pairs_map() -> list(TriePair) = {
    var out : list(TriePair) = [||];
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_at(get_slice_int(64, j, 0));
        let addr = am_at_addr();
        let acc : Account = struct {
                acc_nonce = unsigned(acctmap_at_nonce()),
                acc_balance = am_at_bal(),
                acc_codehash = am_at_chash(),
                acc_storage_root = EMPTY_TRIE_ROOT,
            };
        if not_bool(acc_is_empty(acc)) then {
            out =
                struct {
                    tp_key = to_nibbles(word_to_bytes32(pre_keccak_addr(addr))),
                    tp_val = account_rlp(acc.acc_nonce, acc.acc_balance, storage_root(addr), acc.acc_codehash),
                } ::
                    out
        }
    };
    out
}

/* Post-state root: incremental, reusing untouched accounts from the pre-state
 * trie (compute_state_root_hb, defined after the witness reader + the stack-builder
 * collector machinery). Forward-declared here. */
val compute_state_root_hb : unit -> bits(256)

/* ======================================================================== */
/* (3) stateless witness-backed reads                                       */
/*                                                                          */
/* Stateless verification of the world-state trie from a Merkle witness     */
/* (the node set covering the proof paths), mirroring execution-specs       */
/* witness_state.py. A node database keyed by keccak256(node) (C-backed,    */
/* ffi/host_nodedb.c) backs two operations.                                 */
/*                                                                          */
/* Re-rooting feeds the witness through the layer-(1) builder and checks    */
/* the result against the committed pre-state root, authenticating every    */
/* supplied node at once. Point reads walk the secure trie by               */
/* keccak256(key) nibbles (branch / extension / leaf, hex-prefix paths) to  */
/* the account leaf and decode RLP([nonce, balance, storageRoot,            */
/* codeHash]); storage reads walk the account's storage root the same way.  */
/*                                                                          */
/* A node absent from the witness on a walked path is a missing proof and   */
/* fails closed. Depends on the RLP decoder (rlp_decode.sail) and the SSZ   */
/* witness-list readers (host/io.sail).                                     */
/* ======================================================================== */

/* EMPTY_TRIE_ROOT is defined in host/state.sail (next to KECCAK_EMPTY) so the
 * Account struct's acc_storage_root default can reference it. */
let EMPTY_CODE_HASH : bits(256) = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

let RLP_NONE : RlpField = struct { rf_is_list = false, rf_content = [||], rf_full = [||] }

/* big-endian bytes -> 256-bit word */
function bytes_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(bytes_get(c, k), 256))
    };
    acc
}

/* decode an account leaf value: (nonce, balance, storage_root, code_hash).
 * Defined here (ahead of the witness reader) so both the inline storage harvest
 * and the lazy account read can use it. */
function decode_account(leaf : list(byte)) -> (int, word, bits(256), bits(256)) = {
    let items = rlp_node_items(leaf);
    let nonce = rlp_field_int(list_nth(items, 0, RLP_NONE));
    let balance = rlp_field_word(list_nth(items, 1, RLP_NONE));
    let sr_f = list_nth(items, 2, RLP_NONE);
    let ch_f = list_nth(items, 3, RLP_NONE);
    let sroot = if list_len(sr_f.rf_content) == 0 then {
            EMPTY_TRIE_ROOT
        } else {
            bytes_be256(sr_f.rf_content)
        };
    let chash = if list_len(ch_f.rf_content) == 0 then {
            EMPTY_CODE_HASH
        } else {
            bytes_be256(ch_f.rf_content)
        };
    (nonce, balance, sroot, chash)
}

/* 64 nibbles (MSB-first) of a 256-bit key hash */
function hash_nibbles(h : bits(256)) -> list(int) = {
    var rev : list(int) = [||];
    foreach (i from 0 to 63) {
        rev = unsigned(sail_shiftright(h, 4 * (63 - i))[3 .. 0]) :: rev
    };
    list_reverse(rev)
}

/* hex-prefix decode: (is_leaf, path nibbles) */
function hp_decode(p : list(byte)) -> (bool, list(int)) = {
    let n = list_len(p);
    if n == 0 then {
        (false, [||])
    } else {
        let fb = unsigned(bytes_get(p, 0));
        let flag = ediv_int(fb, 16);
        let is_leaf : bool = emod_int(flag, 4) >= 2;
        let odd : bool = emod_int(flag, 2) == 1;
        var rev : list(int) = [||];
        if odd then {
            rev = emod_int(fb, 16) :: rev
        };
        foreach (i from 1 to (n - 1)) {
            let byte = unsigned(bytes_get(p, i));
            rev = ediv_int(byte, 16) :: rev;
            rev = emod_int(byte, 16) :: rev
        };
        (is_leaf, list_reverse(rev))
    }
}

/* do seg nibbles match key[pos ..] ? */
function nibs_match(key : list(int), pos : int, seg : list(int)) -> bool = {
    let m = list_len(seg);
    var ok : bool = true;
    foreach (k from 0 to (m - 1)) {
        if list_nth(key, pos + k, -1) != list_nth(seg, k, -2) then {
            ok = false
        }
    };
    ok
}

/* node db: keccak(node) -> node bytes (linear scan; witnesses are small) */
/* C-backed node-db (ffi/host_nodedb.c): keccak(node) -> (offset, len) span into
 * the SSZ witness buffer. Replaces a Sail assoc list whose linear node_db_lookup
 * (plus the whole list deep-copied through every recursive walk) made re-rooting
 * O(N^2); this is an O(1)-amortized hash map, so the walk is O(N). The 256-bit
 * key crosses as four big-endian words (k3 most significant); nodedb_sel caches
 * the matched span for the off/len accessors. */
val nodedb_reset = impure { _: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { _: "nodedb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val nodedb_sel = impure { _: "nodedb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val nodedb_sel_off = impure { _: "nodedb_sel_off" } : unit -> bits(64)
val nodedb_sel_len = impure { _: "nodedb_sel_len" } : unit -> bits(64)

/* C-backed witness account-leaf store (ffi/host_nodedb.c): keccak(addr) ->
 * (offset, len) span of the account's leaf node in the SSZ witness buffer.
 * Recorded during the state re-root walk; a stateless account read looks the span
 * up here (O(1)) and decodes the leaf lazily. Same span model as the node-db. */
val acctdb_reset = impure { _: "acctdb_reset" } : unit -> unit
val acctdb_insert = impure { _: "acctdb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val acctdb_sel = impure { _: "acctdb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val acctdb_sel_off = impure { _: "acctdb_sel_off" } : unit -> bits(64)
val acctdb_sel_len = impure { _: "acctdb_sel_len" } : unit -> bits(64)
val acctdb_count = impure { _: "acctdb_count" } : unit -> bits(64)
val acctdb_at = impure { _: "acctdb_at" } : bits(64) -> unit
val acctdb_at_key = impure { _: "acctdb_at_key" } : bits(64) -> bits(64)
val acctdb_at_off = impure { _: "acctdb_at_off" } : unit -> bits(64)
val acctdb_at_len = impure { _: "acctdb_at_len" } : unit -> bits(64)

/* node bytes for keccak `h` from the C node-db, or [] if absent (read straight
 * from the witness buffer by the recorded span -- no per-node Sail copy held). */
function node_db_lookup(h : bits(256)) -> list(byte) = {
    if nodedb_sel(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]) == 0x0000000000000000 then {
        [||]
    } else {
        ssz_bytes(unsigned(nodedb_sel_off()), unsigned(nodedb_sel_len()))
    }
}

/* resolve a child reference: embedded node (full bytes), 32-byte hash, or empty.
 * A 32-byte hash that is absent from the witness is a MISSING node (the prover
 * omitted a node on a walked path) -- fail closed, so a resolved-empty result
 * always means a legitimate exclusion, never a withheld subtree. */
function resolve_child(f : RlpField) -> list(byte) =
    if f.rf_is_list then {
        f.rf_full
    } else {
        if list_len(f.rf_content) == 32 then {
            let node = node_db_lookup(bytes_be256(f.rf_content));

            /* a 32-byte ref absent from the witness means the proof is deficient:
               flag the block invalid (valid=false) and end the walk here, rather
               than aborting -- a missing proof is a NORMAL failure outcome
               (EIP-8025 optional proofs), not a guest crash. */
            if list_len(node) == 0 then {
                k_witness_bad = true
            };
            node
        } else {
            [||]
        }
    }

/* walk the secure trie from `node` following key nibbles; return the leaf value */
function trie_walk(node : list(byte), key : list(int), pos : int, fuel : int) -> list(byte) = {
    if fuel <= 0 | list_len(node) == 0 then {
        [||]
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).rf_content);
            if not_bool(nibs_match(key, pos, seg)) then {
                [||]
            } else {
                if is_leaf then {
                    if pos + list_len(seg) == list_len(key) then {
                        list_nth(items, 1, RLP_NONE).rf_content
                    } else {
                        [||]
                    }
                } else {
                    let child = resolve_child(list_nth(items, 1, RLP_NONE));
                    trie_walk(child, key, pos + list_len(seg), fuel - 1)
                }
            }
        } else {
            if n == 17 then {
                if pos == list_len(key) then {
                    list_nth(items, 16, RLP_NONE).rf_content
                } else {
                    let child = resolve_child(list_nth(items, list_nth(key, pos, 0), RLP_NONE));
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            } else {
                [||]
            }
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

/* ---- witness -> HashBuilder: re-root a partial trie in one pass --------- */
/* Left-to-right DFS over the provided witness trie. Each present leaf is fed
 * to the stack builder (hb_add_leaf) and each child reference whose node is NOT
 * in the witness (a blinded frontier subtree) is fed as a hash (hb_add_branch).
 * A branch slot 0..15 is visited in ascending nibble order, so entries arrive
 * already key-sorted -- exactly what the HashBuilder requires. Re-rooting and
 * comparing to the committed state root validates the whole pre-state at once.
 * The SAME pass, at each account leaf, records the leaf's span into the acctdb
 * (so account reads decode lazily, O(1), keyed by keccak(addr)). The acctdb then
 * drives the storage-harvest pass (harvest_all_storage) -- no Sail leaf list is
 * kept; the C-backed acctdb is the cross-pass enumerator. */

/* 64 nibbles (MSB-first) -> bits(256) */
function nibbles_to_h256(nibs : list(int)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(nibs);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 4), sail_zero_extend(get_slice_int(4, list_nth(nibs, k, 0), 0), 256))
    };
    acc
}

val witness_feed : (list(byte), list(int), int) -> unit

/* feed one child field at nibble-path `key`: descend if present, blind if not */
function witness_feed_child(f : RlpField, key : list(int), fuel : int) -> unit = {
    if f.rf_is_list then {
        witness_feed(f.rf_full, key, fuel)
        /* embedded node */
    } else {
        let c = f.rf_content;
        if list_len(c) == 0 then {
            ()
            /* empty slot */
        } else {
            if list_len(c) == 32 then {
                let node = node_db_lookup(bytes_be256(c));
                if list_len(node) == 0 then {
                    hb_add_branch(key, c)
                    /* blinded frontier */
                } else {
                    witness_feed(node, key, fuel)
                } /* descend */
            }
        }
    }
}

function witness_feed(node : list(byte), prefix : list(int), fuel : int) -> unit = {
    if fuel <= 0 | list_len(node) == 0 then {
        ()
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).rf_content);
            let key = list_append(prefix, seg);
            if is_leaf then {
                let value = list_nth(items, 1, RLP_NONE).rf_content;
                hb_add_leaf(key, value);
                if list_len(key) == 64 then {
                    if hv_storage then {
                        /* storage trie: harvest the slot's pre-value into slotdb,
                           keyed by (hv_acct, keccak(slot)) = (hv_acct, leaf key) */
                        let sv = rlp_str_word(value);
                        slot_insert(hv_acct, nibbles_to_h256(key), sv)
                    } else {
                        /* account leaf: record keccak(addr) -> the leaf node's span
                           for lazy O(1) reads (the leaf is >=32B, hence a hashed node
                           in the node-db -- recover its span via keccak(node)). The
                           account's storage trie is harvested in the second pass
                           (harvest_all_storage), driven by this acctdb. */
                        let ak = nibbles_to_h256(key);
                        let hn = keccak256(node);
                        let _ = nodedb_sel(hn[255 .. 192], hn[191 .. 128], hn[127 .. 64], hn[63 .. 0]);
                        acctdb_insert(
                            ak[255 .. 192],
                            ak[191 .. 128],
                            ak[127 .. 64],
                            ak[63 .. 0],
                            nodedb_sel_off(),
                            nodedb_sel_len(),
                        )
                    }
                }
            } else {
                witness_feed_child(list_nth(items, 1, RLP_NONE), key, fuel - 1)
            } /* extension */
        } else {
            if n == 17 then {
                let v = list_nth(items, 16, RLP_NONE).rf_content; /* branch value: key ends here */
                if list_len(v) > 0 then {
                    hb_add_leaf(prefix, v)
                }; /* (empty for secure tries) */
                foreach (i from 0 to 15) {
                    witness_feed_child(list_nth(items, i, RLP_NONE), list_append(prefix, [|i|]), fuel - 1)
                }
            }
        }
    }
}
termination_measure witness_feed(node, prefix, fuel) = fuel
termination_measure witness_feed_child(f, key, fuel) = fuel

/* re-root the trie at `root` from the witness via the stack builder. The walk
 * also records each account's leaf span into the acctdb (witness_feed); the
 * storage tries are then authenticated + harvested in a second pass driven by the
 * acctdb (harvest_all_storage), once the state builder is free. */
function witness_reroot(root : bits(256)) -> bits(256) = {
    hb_reset();
    acctdb_reset();
    let rootnode = node_db_lookup(root);
    if list_len(rootnode) == 0 then {
        keccak256([|0x80|])
    } else {
        witness_feed(rootnode, [||], 256);
        hb_root()
    }
}

/* ---- account / storage resolution against the witness ------------------ */

/* populate the C node-db from witness.state: keccak(node) -> span. The node is
 * materialized once here to hash it; the recorded span (not the bytes) is what
 * the walk reads back, so no per-node Sail copy is retained. */
function build_node_db() -> unit = {
    nodedb_reset();
    let n = wit_n_state();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_witness_node_span(i);
        let h = keccak256(ssz_bytes(off, len));
        nodedb_insert(
            h[255 .. 192],
            h[191 .. 128],
            h[127 .. 64],
            h[63 .. 0],
            get_slice_int(64, off, 0),
            get_slice_int(64, len, 0),
        )
    }
}

/* the parent header's state_root (field 3) = pre-state root. The witness headers
 * form a chain header[0] (oldest) .. header[n-1] (newest), where header[i]'s
 * parent is header[i-1] (validated by headers_ok). The block being validated
 * extends the NEWEST, so its parent is header[n-1] -- NOT header[0]. With a
 * single header they coincide; when BLOCKHASH/EIP-2935 pulls in older headers
 * (EIP-8025 witness header collection) the parent is the last one. */
function parent_state_root() -> bits(256) = {
    let items = rlp_node_items(ssz_witness_header(wit_n_headers() - 1));
    bytes_be256(list_nth(items, 3, RLP_NONE).rf_content)
}

/* Stateless storage resolver (declared in host/state.sail): the (a, s) pre-state
 * value from the witness. The slot store holds the present pre-state slots
 * harvested during the witness pass, so a hit is an O(1) lookup. On a MISS the
 * value is resolved (and its presence/absence PROVEN) by a Merkle walk of the
 * account's storage trie from its authenticated pre-state root:
 *   - the walk reaches the leaf            => present, use that value;
 *   - the walk reaches a diverging/empty
 *     terminus through present nodes        => PROVEN ABSENT => 0;
 *   - the walk hits a blinded (witness-
 *     absent) node on the path              => trie_walk asserts "witness missing
 *                                              trie node" => fail closed.
 * The resolved value (or proven-absent 0) is cached in the slot store, so repeat
 * reads are O(1) -- one keccak (memoized) and one walk per slot, never per read. */
function stateless_storage(a : address, s : word) -> word = {
    let ak = pre_keccak_addr(a);
    let sk = pre_keccak_slot(s);
    if slotdb_sel(
        ak[255 .. 192],
        ak[191 .. 128],
        ak[127 .. 64],
        ak[63 .. 0],
        sk[255 .. 192],
        sk[191 .. 128],
        sk[127 .. 64],
        sk[63 .. 0],
    ) !=
        0x0000000000000000 then {
        slot_selword()
    } else {
        let sroot = kget(a).acc_storage_root;
        let leaf = trie_walk(node_db_lookup(sroot), hash_nibbles(sk), 0, 256);
        let v : word = if list_len(leaf) > 0 then {
                rlp_str_word(leaf)
            } else {
                sail_zeros(256)
            };
        slot_insert(ak, sk, v); /* cache the proven value / proven-absent 0 */
        v
    }
}

/* Authenticate + harvest one account's storage trie: re-root it from the witness
 * (witness_feed, with hv_storage routing leaves into the slot store under `ak`)
 * and assert the reconstructed root equals the account's committed storage root. */
/* EIP-8025 (optional proofs): the witness omits an account's storage proof when
 * the block never touches its storage. Such an account has a non-empty committed
 * storage_root whose root node is ABSENT from the witness node-db -- there is
 * nothing to harvest or re-root, and the storage is unchanged, so its post-state
 * storage_root stays the committed value. */
function storage_proof_omitted(sroot : bits(256)) -> bool =
    (list_len(node_db_lookup(sroot)) == 0) & (sroot != EMPTY_TRIE_ROOT)

function harvest_storage(ak : bits(256), sroot : bits(256)) -> unit = {
    if storage_proof_omitted(sroot) then {
        ()
    } else {
        hb_reset();
        hv_storage = true;
        hv_acct = ak;
        witness_feed(node_db_lookup(sroot), [||] : list(int), 256);
        hv_storage = false;

        /* a storage proof that does not re-root to the account's committed
           storage_root is deficient -> graceful invalid (not an abort). */
        if hb_root() != sroot then {
            k_witness_bad = true
        }
    }
}

/* 256-bit keccak(addr) of the current acctdb_at row */
function acctdb_at_acct() -> bits(256) =
    append(
        acctdb_at_key(get_slice_int(64, 3, 0)),
        append(
            acctdb_at_key(get_slice_int(64, 2, 0)),
            append(acctdb_at_key(get_slice_int(64, 1, 0)), acctdb_at_key(get_slice_int(64, 0, 0))),
        ),
    )

/* Storage-harvest pass: for every account leaf recorded in the acctdb during the
 * state re-root, authenticate + harvest its storage trie into the slot store.
 * Runs after witness_reroot (the state HashBuilder is free), driven by the acctdb
 * (the C-backed cross-pass enumerator -- no Sail leaf list). After this the slot
 * store holds the full, witness-proven pre-state storage of every account: the
 * base for stateless reads and for the post-state root (storage_pairs). */
function harvest_all_storage() -> unit = {
    let n = unsigned(acctdb_count());
    foreach (i from 0 to (n - 1)) {
        acctdb_at(get_slice_int(64, i, 0));
        let ak = acctdb_at_acct();
        let node = ssz_bytes(unsigned(acctdb_at_off()), unsigned(acctdb_at_len()));
        let value = list_nth(rlp_node_items(node), 1, RLP_NONE).rf_content;
        let (_, _, sroot, _) = decode_account(value);
        if sroot != EMPTY_TRIE_ROOT then {
            harvest_storage(ak, sroot)
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch. A hit in the acctdb (account leaves recorded during the witness
 * re-root) materializes from the leaf span. On a MISS, presence/absence is PROVEN
 * by a Merkle walk of the state trie from the authenticated pre-state root:
 *   - the walk reaches the leaf             => present, decode + seed;
 *   - reaches a diverging/empty terminus
 *     through present nodes                  => PROVEN ABSENT => empty account;
 *   - hits a blinded (witness-absent) node   => trie_walk asserts => fail closed.
 * So a not-materialized account is never silently treated as empty -- its
 * absence is a witness-proven exclusion. The result is cached raw in the acctmap
 * (k_seed_account_hashed), so subsequent kget(a) is an O(1) raw read. */
function stateless_account_load(a : address) -> unit = {
    let ak = pre_keccak_addr(a);
    let value : list(byte) = if acctdb_sel(ak[255 .. 192], ak[191 .. 128], ak[127 .. 64], ak[63 .. 0]) !=
            0x0000000000000000 then {
            list_nth(rlp_node_items(ssz_bytes(unsigned(acctdb_sel_off()), unsigned(acctdb_sel_len()))), 1, RLP_NONE).rf_content
        } else {
            trie_walk(node_db_lookup(parent_state_root()), hash_nibbles(ak), 0, 256)
        };
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        k_seed_account_hashed(a, nonce, bal, sroot, chash)
    } else {
        k_seed_account_hashed(a, 0, sail_zeros(256), EMPTY_TRIE_ROOT, KECCAK_EMPTY)
    }
}

/* ---- incremental post-state storage root (blinded-subtree reuse) -------------
 * The post-state storage root of an account, computed by REUSING the account's
 * untouched (blinded) pre-state subtrees instead of re-enumerating them. The
 * present + written leaves come from storage_pairs (harvested pre-state slots
 * merged with overlay writes); a DFS of the account's witness storage trie
 * collects every BLINDED child as a subtree to reuse by hash. Leaves and blinded
 * subtrees are folded together in ascending key order by the stack builder,
 * which yields the correct root even when most of the trie is blinded -- the
 * case a flat leaf enumeration gets wrong (it silently drops the blinded slots).
 * For a fully materialized witness no child is blinded, so this reduces exactly
 * to the old hb_root_of(sort_pairs(storage_pairs(.))). */

/* a stack-builder feed entry: a present/updated leaf (key -> rlp value) or a
 * BLINDED subtree reused by hash (prefix -> 32-byte child ref). */
struct CollEntry = { ce_key : list(int), ce_branch : bool, ce_bytes : list(byte) }

/* blinded subtrees collected while walking an account's witness storage trie */
register cb_acc : list(CollEntry) = [||]

val cb_feed : (list(byte), list(int), int) -> unit

/* one storage child: descend if present in the witness, RECORD it (prefix ->
 * hash) if blinded. Present leaves are NOT recorded -- they come via
 * storage_pairs (slotdb), so only blinded subtrees need reuse here. */
function cb_feed_child(f : RlpField, key : list(int), fuel : int) -> unit = {
    if f.rf_is_list then {
        cb_feed(f.rf_full, key, fuel)
    } else {
        let c = f.rf_content;
        if list_len(c) == 32 then {
            let node = node_db_lookup(bytes_be256(c));
            if list_len(node) == 0 then {
                cb_acc = struct { ce_key = key, ce_branch = true, ce_bytes = c } :: cb_acc
            } else {
                cb_feed(node, key, fuel)
            }
        }
    }
}

function cb_feed(node : list(byte), prefix : list(int), fuel : int) -> unit = {
    if fuel <= 0 | list_len(node) == 0 then {
        ()
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).rf_content);
            if is_leaf then {
                () /* present leaf -> already a storage_pairs leaf */
            } else {
                cb_feed_child(list_nth(items, 1, RLP_NONE), list_append(prefix, seg), fuel - 1)
            }
        } else {
            if n == 17 then {
                foreach (i from 0 to 15) {
                    cb_feed_child(list_nth(items, i, RLP_NONE), list_append(prefix, [|i|]), fuel - 1)
                }
            }
        }
    }
}
termination_measure cb_feed(node, prefix, fuel) = fuel
termination_measure cb_feed_child(f, key, fuel) = fuel

/* sort feed entries by nibble key (the stack builder requires ascending keys);
 * leaves and blinded subtrees are disjoint in coverage, so a total order on the
 * key prefix is well-defined. */
function ce_insert(p : CollEntry, ps : list(CollEntry)) -> list(CollEntry) =
    match ps {
        [||]   => [|p|],
        q :: r => if nib_lt(p.ce_key, q.ce_key) then {
            p :: ps
        } else {
            q :: ce_insert(p, r)
        },
    }
function ce_sort(ps : list(CollEntry)) -> list(CollEntry) =
    match ps {
        [||]   => [||],
        q :: r => ce_insert(q, ce_sort(r)),
    }
function ce_concat(xs : list(CollEntry), ys : list(CollEntry)) -> list(CollEntry) =
    match xs {
        [||]   => ys,
        x :: r => x :: ce_concat(r, ys),
    }
function pairs_to_entries(ps : list(TriePair)) -> list(CollEntry) =
    match ps {
        [||]   => [||],
        p :: r => struct { ce_key = p.tp_key, ce_branch = false, ce_bytes = p.tp_val } :: pairs_to_entries(r),
    }

/* fold a sorted (leaf | blinded-branch) sequence into a root via the stack builder */
function feed_entries(entries : list(CollEntry)) -> bits(256) = {
    hb_reset();
    var es : list(CollEntry) = entries;
    var more : bool = true;
    while more do {
        match es {
            e :: r => {
                if e.ce_branch then {
                    hb_add_branch(e.ce_key, e.ce_bytes)
                } else {
                    hb_add_leaf(e.ce_key, e.ce_bytes)
                };
                es = r
            },
            [||] => more = false,
        }
    };
    hb_root()
}

function storage_root(a : address) -> bits(256) = {
    let sroot = kget(a).acc_storage_root;
    if storage_proof_omitted(sroot) then {
        /* EIP-8025: untouched storage, proof omitted -> reuse committed root */
        sroot
    } else {
        let leaves = pairs_to_entries(storage_pairs(a));
        cb_acc = [||];
        cb_feed(node_db_lookup(sroot), [||] : list(int), 256);
        feed_entries(ce_sort(ce_concat(leaves, cb_acc)))
    }
}

/* ---- incremental post-state root: untouched-account reuse --------------------
 * The post-state account leaves come from state_pairs_map (touched + created;
 * EIP-161-empty dropped). Pre-existing accounts the block never touched are NOT in
 * the acctmap -- their raw address is unknown, only keccak(addr) is witnessed -- so
 * a flat enumeration drops them. sb_feed walks the pre-state trie and REUSES every
 * untouched account: its unchanged leaf, or a blinded subtree by hash, folded with
 * the fresh leaves via the stack builder. The touched set is exactly the acctmap
 * keccaks -- every touched pre-state account stays in the acctmap (reads materialize
 * it; emptied / pre-existing-selfdestructed accounts remain), and created-then-
 * destroyed accounts were never in the pre-state trie. For a fully materialized
 * witness every present leaf is touched and nothing is blinded, so this reduces to
 * a flat hb_root_of(sort_pairs(state_pairs_map())). */

/* 64-nibble trie key -> the 256-bit keccak(addr) it encodes */
function nibs_to_h256(nibs : list(int)) -> bits(256) = {
    var v : int = 0;
    var rest : list(int) = nibs;
    var more : bool = true;
    while more do {
        match rest {
            x :: r => {
                v = v * 16 + x;
                rest = r
            },
            [||] => more = false,
        }
    };
    word_of_int(v)
}

/* keccak(addr) of every account in the acctmap (= the touched set) */
register sb_touched : list(bits(256)) = [||]
function build_state_touched() -> unit = {
    sb_touched = [||];
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_at(get_slice_int(64, j, 0));
        sb_touched = pre_keccak_addr(am_at_addr()) :: sb_touched
    }
}

/* untouched-account leaves + blinded subtrees collected over the pre-state trie */
register sb_acc : list(CollEntry) = [||]
val sb_feed : (list(byte), list(int), int) -> unit

/* one state child: descend if present, RECORD a blinded child as a reuse branch */
function sb_feed_child(f : RlpField, key : list(int), fuel : int) -> unit = {
    if f.rf_is_list then {
        sb_feed(f.rf_full, key, fuel)
    } else {
        let c = f.rf_content;
        if list_len(c) == 32 then {
            let node = node_db_lookup(bytes_be256(c));
            if list_len(node) == 0 then {
                sb_acc = struct { ce_key = key, ce_branch = true, ce_bytes = c } :: sb_acc
            } else {
                sb_feed(node, key, fuel)
            }
        }
    }
}

function sb_feed(node : list(byte), prefix : list(int), fuel : int) -> unit = {
    if fuel <= 0 | list_len(node) == 0 then {
        ()
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).rf_content);
            let key = list_append(prefix, seg);
            if is_leaf then {
                /* present leaf: touched -> re-emitted by state_pairs_map (skip);
                   untouched -> REUSE its unchanged pre-state account leaf */
                if not_bool(h256_in(sb_touched, nibs_to_h256(key))) then {
                    sb_acc =
                        struct { ce_key = key, ce_branch = false, ce_bytes = list_nth(items, 1, RLP_NONE).rf_content } ::
                            sb_acc
                }
            } else {
                sb_feed_child(list_nth(items, 1, RLP_NONE), key, fuel - 1)
            }
        } else {
            if n == 17 then {
                foreach (i from 0 to 15) {
                    sb_feed_child(list_nth(items, i, RLP_NONE), list_append(prefix, [|i|]), fuel - 1)
                }
            }
        }
    }
}
termination_measure sb_feed(node, prefix, fuel) = fuel
termination_measure sb_feed_child(f, key, fuel) = fuel

function compute_state_root_hb() -> bits(256) = {
    let leaves = pairs_to_entries(state_pairs_map());
    build_state_touched();
    sb_acc = [||];
    sb_feed(node_db_lookup(parent_state_root()), [||] : list(int), 256);
    feed_entries(ce_sort(ce_concat(leaves, sb_acc)))
}

/* EIP-7928 block-access-list item count over the RLP-encoded BlockAccessList (a
 * ByteList in the payload): rlp([ [address, storage_changes, storage_reads,
 * balance_changes, nonce_changes, code_changes], ... ]). Item count =
 * #accounts + sum over accounts of (#storage_changes + #storage_reads), where
 * storage_changes (RLP item 1) is a list of [slot, changes] pairs and
 * storage_reads (item 2) is a list of slots. EELS
 * validate_block_access_list_gas_limit rejects the block if this exceeds
 * block_gas_limit / BLOCK_ACCESS_LIST_ITEM (2000). */
function bal_account_items(acc : list(byte)) -> int = {
    let items = rlp_node_items(acc);
    let sc = list_nth(items, 1, RLP_NONE);
    let sr = list_nth(items, 2, RLP_NONE);
    1 + list_len(rlp_node_items(sc.rf_full)) + list_len(rlp_node_items(sr.rf_full))
}
function bal_item_count() -> int = {
    let b = pl_bal_start();
    let e = payload_end();
    if b >= e then {
        0
    } else {
        var total : int = 0;
        var rest : list(RlpField) = rlp_node_items(ssz_bytes(b, e - b));
        var more : bool = true;
        while more do {
            match rest {
                a :: r => {
                    total = total + bal_account_items(a.rf_full);
                    rest = r
                },
                [||] => more = false,
            }
        };
        total
    }
}
