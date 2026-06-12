/* ======================================================================== */
/* Witness-backed secure-trie reads (mirrors execution-specs forks/amsterdam    */
/* witness_state.py): a node db keyed by keccak(node), a secure-trie walk by    */
/* keccak(key) nibbles over branch/extension/leaf nodes, and account-leaf RLP   */
/* decode. Storage reads reuse the same walk against an account's storage root. */
/*                                                                           */
/* Depends on rlp_decode.sail (RlpField/rlp_node_items) + keccak (el_ir.sail) + */
/* the SSZ witness-list readers (ssz.sail). Pure given the node db; the db is   */
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
    rev = unsigned((sail_shiftright(h, 4 * (63 - i)))[3 .. 0]) :: rev
  };
  list_reverse(rev)
}

/* hex-prefix decode: (is_leaf, path nibbles) */
function hp_decode(p : list(byte)) -> (bool, list(int)) = {
  let n = list_len(p);
  if n == 0 then (false, [||])
  else {
    let fb = unsigned(bytes_get(p, 0));
    let flag = ediv_int(fb, 16);
    let is_leaf : bool = emod_int(flag, 4) >= 2;
    let odd : bool = emod_int(flag, 2) == 1;
    var rev : list(int) = [||];
    if odd then rev = emod_int(fb, 16) :: rev else ();
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
    if list_nth(key, pos + k, -1) != list_nth(seg, k, -2) then ok = false else ()
  };
  ok
}

/* node db: keccak(node) -> node bytes (linear scan; witnesses are small) */
function node_db_lookup(db : list((bits(256), list(byte))), h : bits(256)) -> list(byte) = {
  let n = list_len(db);
  var res : list(byte) = [||];
  foreach (i from 0 to (n - 1)) {
    let (k, v) = list_nth(db, i, (sail_zeros(256), [||] : list(byte)));
    if k == h then res = v else ()
  };
  res
}

/* resolve a child reference: embedded node (full bytes), 32-byte hash, or empty.
 * A 32-byte hash that is absent from the witness is a MISSING node (the prover
 * omitted a node on a walked path) -- fail closed, so a resolved-empty result
 * always means a legitimate exclusion, never a withheld subtree. */
function resolve_child(db : list((bits(256), list(byte))), f : RlpField) -> list(byte) =
  if f.rf_is_list then f.rf_full
  else if list_len(f.rf_content) == 32 then {
    let node = node_db_lookup(db, bytes_be256(f.rf_content));
    assert(list_len(node) > 0, "witness missing trie node");
    node
  }
  else [||]

/* walk the secure trie from `node` following key nibbles; return the leaf value */
function trie_walk(db : list((bits(256), list(byte))), node : list(byte),
                   key : list(int), pos : int, fuel : int) -> list(byte) = {
  if fuel <= 0 | list_len(node) == 0 then [||]
  else {
    let items = rlp_node_items(node);
    let n = list_len(items);
    if n == 2 then {
      let (is_leaf, seg) = hp_decode((list_nth(items, 0, RLP_NONE)).rf_content);
      if not_bool(nibs_match(key, pos, seg)) then [||]
      else if is_leaf then {
        if pos + list_len(seg) == list_len(key) then (list_nth(items, 1, RLP_NONE)).rf_content
        else [||]
      } else {
        let child = resolve_child(db, list_nth(items, 1, RLP_NONE));
        trie_walk(db, child, key, pos + list_len(seg), fuel - 1)
      }
    } else if n == 17 then {
      if pos == list_len(key) then (list_nth(items, 16, RLP_NONE)).rf_content
      else {
        let child = resolve_child(db, list_nth(items, list_nth(key, pos, 0), RLP_NONE));
        trie_walk(db, child, key, pos + 1, fuel - 1)
      }
    } else [||]
  }
}
termination_measure trie_walk(db, node, key, pos, fuel) = fuel

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
    if k == h then res = v else ()
  };
  res
}

val witness_feed : (list((bits(256), list(byte))), list(byte), list(int), int) -> unit

/* feed one child field at nibble-path `key`: descend if present, blind if not */
function witness_feed_child(db : list((bits(256), list(byte))), f : RlpField,
                           key : list(int), fuel : int) -> unit = {
  if f.rf_is_list then witness_feed(db, f.rf_full, key, fuel)        /* embedded node */
  else {
    let c = f.rf_content;
    if list_len(c) == 0 then ()                                       /* empty slot */
    else if list_len(c) == 32 then {
      let node = node_db_lookup(db, bytes_be256(c));
      if list_len(node) == 0 then hb_add_branch(key, c)               /* blinded frontier */
      else witness_feed(db, node, key, fuel)                          /* descend */
    } else ()
  }
}

function witness_feed(db : list((bits(256), list(byte))), node : list(byte),
                      prefix : list(int), fuel : int) -> unit = {
  if fuel <= 0 | list_len(node) == 0 then ()
  else {
    let items = rlp_node_items(node);
    let n = list_len(items);
    if n == 2 then {
      let (is_leaf, seg) = hp_decode((list_nth(items, 0, RLP_NONE)).rf_content);
      let key = list_append(prefix, seg);
      if is_leaf then {
        let value = (list_nth(items, 1, RLP_NONE)).rf_content;
        hb_add_leaf(key, value);
        if list_len(key) == 64 then witness_leaves = (nibbles_to_h256(key), value) :: witness_leaves else ()
      }
      else witness_feed_child(db, list_nth(items, 1, RLP_NONE), key, fuel - 1)   /* extension */
    } else if n == 17 then {
      let v = (list_nth(items, 16, RLP_NONE)).rf_content;             /* branch value: key ends here */
      if list_len(v) > 0 then hb_add_leaf(prefix, v) else ();         /* (empty for secure tries) */
      foreach (i from 0 to 15) {
        witness_feed_child(db, list_nth(items, i, RLP_NONE), list_append(prefix, [| i |]), fuel - 1)
      }
    } else ()
  }
}
termination_measure witness_feed(db, node, prefix, fuel) = fuel
termination_measure witness_feed_child(db, f, key, fuel) = fuel

/* re-root the trie at `root` from the witness via the stack builder */
function witness_reroot(db : list((bits(256), list(byte))), root : bits(256)) -> bits(256) = {
  hb_reset();
  witness_leaves = [||];
  let rootnode = node_db_lookup(db, root);
  if list_len(rootnode) == 0 then keccak256([| 0x80 |])
  else { witness_feed(db, rootnode, [||], 256); hb_root() }
}

/* ---- account / storage resolution against the witness ------------------ */

/* decode an account leaf: (nonce, balance, storage_root, code_hash) */
function decode_account(leaf : list(byte)) -> (int, word, bits(256), bits(256)) = {
  let items = rlp_node_items(leaf);
  let nonce   = rlp_field_int(list_nth(items, 0, RLP_NONE));
  let balance = rlp_field_word(list_nth(items, 1, RLP_NONE));
  let sr_f = list_nth(items, 2, RLP_NONE);
  let ch_f = list_nth(items, 3, RLP_NONE);
  let sroot = if list_len(sr_f.rf_content) == 0 then EMPTY_TRIE_ROOT else bytes_be256(sr_f.rf_content);
  let chash = if list_len(ch_f.rf_content) == 0 then EMPTY_CODE_HASH else bytes_be256(ch_f.rf_content);
  (nonce, balance, sroot, chash)
}

/* build the node db from witness.state (keccak(node) -> node) */
function build_node_db() -> list((bits(256), list(byte))) = {
  let n = wit_n_state();
  var db : list((bits(256), list(byte))) = [||];
  foreach (i from 0 to (n - 1)) {
    let nb = ssz_witness_node(i);
    db = (keccak256(nb), nb) :: db
  };
  db
}

/* the parent header's state_root (witness.headers[0] field 3) = pre-state root */
function parent_state_root() -> bits(256) = {
  let items = rlp_node_items(ssz_witness_header(0));
  bytes_be256((list_nth(items, 3, RLP_NONE)).rf_content)
}

/* resolve an account leaf (RLP) for `addr`, or [] if absent */
function witness_get_account(db : list((bits(256), list(byte))), state_root : bits(256),
                             addr : address) -> list(byte) = {
  let key = hash_nibbles(keccak256(address_to_bytes(addr)));
  trie_walk(db, node_db_lookup(db, state_root), key, 0, 80)
}

/* resolve a storage slot value for `slot` under `storage_root` (U256, or 0) */
function witness_get_storage(db : list((bits(256), list(byte))), sroot : bits(256),
                             slot : word) -> word = {
  if sroot == EMPTY_TRIE_ROOT then sail_zeros(256)
  else {
    let key = hash_nibbles(keccak256(word_to_bytes32(slot)));
    let leaf = trie_walk(db, node_db_lookup(db, sroot), key, 0, 80);
    if list_len(leaf) == 0 then sail_zeros(256)
    else rlp_field_word(struct { rf_is_list = false, rf_content = leaf, rf_full = leaf })
  }
}

/* ---- S2b self-check: resolve the two fixture accounts from the witness -- */
function witness_selfcheck() -> unit = {
  let db = build_node_db();
  let sr = parent_state_root();

  let cleaf = witness_get_account(db, sr, 0x000000000000000000000000000000000000c0de);
  assert(list_len(cleaf) > 0, "contract account resolved from witness");
  let (cn, cb, _, cch) = decode_account(cleaf);
  assert(cn == 1, "contract nonce == 1");
  assert(unsigned(cb) == 0, "contract balance == 0");
  assert(cch == 0xac200f16708c3b5c9c2ebffbb1bc40de8df337d6389e49f6425292e9ae777d7e, "contract code hash");

  let eleaf = witness_get_account(db, sr, 0x00000000000000000000000000000000000005e0);
  assert(list_len(eleaf) > 0, "eoa account resolved from witness");
  let (en, eb, _, ech) = decode_account(eleaf);
  assert(en == 0, "eoa nonce == 0");
  assert(unsigned(eb) == 1000000000000000000, "eoa balance == 1e18");
  assert(ech == EMPTY_CODE_HASH, "eoa empty code hash")
}

/* self-check: the witness must re-root to the committed parent state root
 * (defined after build_node_db/parent_state_root, which it depends on). */
function hb_witness_selfcheck() -> unit = {
  let db = build_node_db();
  let sr = parent_state_root();
  assert(witness_reroot(db, sr) == sr, "witness re-roots to parent.state_root")
}
