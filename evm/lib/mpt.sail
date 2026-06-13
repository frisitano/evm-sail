/* ======================================================================== */
/* Merkle-Patricia Trie: root builder + the Ethereum state trie              */
/*                                                                           */
/* Two layers, one trie. (1) A stack-based MPT root builder (the alloy-trie  */
/* HashBuilder port): feed key-sorted (nibble-key, value) leaves and read the */
/* root; nodes < 32 bytes embed, else they hash (child_ref); hex-prefix path  */
/* encoding per YP Appendix C. (2) The Ethereum state trie over the kernel's  */
/* accounts: key = keccak(address), value = rlp[nonce, balance, storageRoot,  */
/* codeHash], with per-account storage roots over keccak(slot) -> rlp(value)  */
/* and EIP-161-empty accounts excluded. compute_state_root_hb authenticates a */
/* block's post-state. Self-contained -> compiles to the RISC-V target.       */
/*                                                                           */
/* (Stateless witness-backed READS of this trie live in lib/mpt_witness.sail, */
/* which additionally depends on the zkVM-only SSZ input layer.)             */
/* ======================================================================== */

/* ===================== (1) the stack root builder ====================== */

/* Stack-based MPT root builder (alloy-trie HashBuilder port)                 */
/*                                                                           */
/* Mirrors alloy-trie 0.9.5 `HashBuilder`: builds a trie root bottom-up from  */
/* sorted (key-nibbles, value) leaves using an explicit stack of child-refs   */
/* plus per-depth state masks. `update` collapses the stack when the next key */
/* diverges, emitting leaf / extension / branch RLP nodes (hashed via         */
/* child_ref == alloy `from_rlp`: inline if <32 bytes, else 0xa0++keccak).     */
/* Blinded frontier subtrees enter via hb_add_branch (push 0xa0++hash without  */
/* descending) -- this is what lets a *partial* witness trie be re-rooted and  */
/* checked against a committed state root in one pass (stateless validation).  */
/*                                                                           */
/* Reuses mpt.sail: hp_compact, child_ref, rlp_bytes/rlp_list, keccak256.      */
/* Self-contained (local byte/empty-root helpers) so it sits in the core model and */
/* can be differentially tested against the recursive `trie_root` oracle.      */
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

/* reference to a child node: inline if its RLP < 32 bytes, else its keccak
 * (alloy `from_rlp`; YP: nodes < 32 bytes are embedded, otherwise hashed). */
function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
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
        assert(len < list_len(current), "hb_update: len < current.len");
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

/* ================== (2) the Ethereum state trie ======================= */

/* Ethereum state root (post-state authentication) in Sail                   */
/*                                                                           */
/* Builds the secure state trie over the kernel's accounts and computes its   */
/* root: key = keccak(address), value = rlp([nonce, balance, storageRoot,     */
/* codeHash]). Per-account storage root = secure trie over keccak(slot) ->    */
/* rlp(value) (zero slots omitted). EIP-161 empty accounts are excluded.      */
/* compute_state_root() == a block's post-state root authenticates execution. */
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

/* secure-trie pairs for one account's non-zero storage, enumerated from the
 * flattened C overlay map (cold path: only --root runs / block authentication). */
function storage_pairs(a : address) -> list(TriePair) = {
    var out : list(TriePair) = [||];
    let n = storage_count();
    foreach (j from 0 to (n - 1)) {
        let (ea, es, ev) = storage_entry(j);
        if ea == a & not_bool(word_is_zero(ev)) then {
            out =
                struct {
                    tp_key = to_nibbles(word_to_bytes32(keccak256(word_to_bytes32(es)))),
                    tp_val = rlp_int(unsigned(ev)),
                } ::
                    out
        }
    };
    out
}
function storage_root(a : address) -> bits(256) = hb_root_of(sort_pairs(storage_pairs(a)))

function acc_is_empty(acc : Account) -> bool =
    (acc.acc_codehash == KECCAK_EMPTY) & acc.acc_nonce == 0 & word_is_zero(acc.acc_balance)

/* secure-trie pairs for all non-empty accounts */
function state_pairs(entries : list(AccountEntry)) -> list(TriePair) =
    match entries {
        [||]   => [||],
        e :: r => if acc_is_empty(e.ae_acc) then {
            state_pairs(r)
        } else {
            struct {
                tp_key = to_nibbles(word_to_bytes32(keccak256(address_to_bytes(e.ae_addr)))),
                tp_val =
                    account_rlp(
                        e.ae_acc.acc_nonce,
                        e.ae_acc.acc_balance,
                        storage_root(e.ae_addr),
                        e.ae_acc.acc_codehash,
                    ),
            } ::
                state_pairs(r)
        },
    }

/* Post-state root via the alloy-trie stack builder: feed the post-execution
 * account leaves (key = keccak(addr), value = rlp(post-account); created accounts
 * included, EIP-161-empty dropped) in sorted key order. The untouched frontier --
 * blinded sibling subtrees for accounts the block never touches -- folds in via
 * hb_add_branch (validated in stage 2); it is empty for a fixture whose whole
 * state is materialised. Same stack machinery as the pre-state validation. */
function compute_state_root_hb() -> bits(256) = hb_root_of(sort_pairs(state_pairs(k_accounts)))

/* ============ (3) stateless witness-backed reads of the trie =========== */
/* feed witness nodes, re-root (via the builder above) to authenticate the   */
/* pre-state, and resolve account/storage values by walking the witness.     */

/* Witness-backed secure-trie reads (mirrors execution-specs forks/amsterdam    */
/* witness_state.py): a node db keyed by keccak(node), a secure-trie walk by    */
/* keccak(key) nibbles over branch/extension/leaf nodes, and account-leaf RLP   */
/* decode. Storage reads reuse the same walk against an account's storage root. */
/*                                                                           */
/* Depends on rlp_decode.sail (RlpField/rlp_node_items) + keccak (evm.sail) + */
/* the SSZ witness-list readers (io.sail). Pure given the node db; the db is   */
/* built once from witness.state.                                              */
/* ======================================================================== */

let EMPTY_TRIE_ROOT : bits(256) = 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421
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
            assert(list_len(node) > 0, "witness missing trie node");
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
 * comparing to the committed state root validates the whole pre-state at once;
 * the same pass is where Stage 4 harvests authenticated leaves into a flat store. */
/* flat store of authenticated leaves harvested during the re-root pass:
 * (full 64-nibble hashed key -> leaf value). Bound to the validated root, so a
 * present entry is a witness-proven value. Account reads are served from here. */
register witness_leaves : list((bits(256), list(byte))) = [||]

/* 64 nibbles (MSB-first) -> bits(256) */
function nibbles_to_h256(nibs : list(int)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(nibs);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 4), sail_zero_extend(get_slice_int(4, list_nth(nibs, k, 0), 0), 256))
    };
    acc
}

/* leaf value for a hashed key from the harvested store ([||] if not present) */
function witness_store_leaf(h : bits(256)) -> list(byte) = {
    var res : list(byte) = [||];
    let n = list_len(witness_leaves);
    foreach (i from 0 to (n - 1)) {
        let (k, v) = list_nth(witness_leaves, i, (sail_zeros(256), [||] : list(byte)));
        if k == h then {
            res = v
        }
    };
    res
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
                    witness_leaves = (nibbles_to_h256(key), value) :: witness_leaves
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

/* re-root the trie at `root` from the witness via the stack builder */
function witness_reroot(root : bits(256)) -> bits(256) = {
    hb_reset();
    witness_leaves = [||];
    let rootnode = node_db_lookup(root);
    if list_len(rootnode) == 0 then {
        keccak256([|0x80|])
    } else {
        witness_feed(rootnode, [||], 256);
        hb_root()
    }
}

/* ---- account / storage resolution against the witness ------------------ */

/* decode an account leaf: (nonce, balance, storage_root, code_hash) */
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

/* the parent header's state_root (witness.headers[0] field 3) = pre-state root */
function parent_state_root() -> bits(256) = {
    let items = rlp_node_items(ssz_witness_header(0));
    bytes_be256(list_nth(items, 3, RLP_NONE).rf_content)
}

/* resolve an account leaf (RLP) for `addr`, or [] if absent */
function witness_get_account(state_root : bits(256), addr : address) -> list(byte) = {
    let key = hash_nibbles(keccak256(address_to_bytes(addr)));
    trie_walk(node_db_lookup(state_root), key, 0, 80)
}

/* resolve a storage slot value for `slot` under `storage_root` (U256, or 0) */
function witness_get_storage(sroot : bits(256), slot : word) -> word = {
    if sroot == EMPTY_TRIE_ROOT then {
        sail_zeros(256)
    } else {
        let key = hash_nibbles(keccak256(word_to_bytes32(slot)));
        let leaf = trie_walk(node_db_lookup(sroot), key, 0, 80);
        if list_len(leaf) == 0 then {
            sail_zeros(256)
        } else {
            rlp_field_word(struct { rf_is_list = false, rf_content = leaf, rf_full = leaf })
        }
    }
}
