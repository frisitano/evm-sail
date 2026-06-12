/* ======================================================================== */
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
function hb_ltake forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
  var out : list('a) = [||];
  var rest : list('a) = xs;
  var i : int = 0;
  while i < n do { match rest { h :: t => { out = h :: out; rest = t }, [||] => () }; i = i + 1 };
  list_reverse(out)
}
function hb_ldrop forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
  var rest : list('a) = xs;
  var i : int = 0;
  while i < n do { match rest { _ :: t => rest = t, [||] => () }; i = i + 1 };
  rest
}

/* ---- bit / mask helpers (16-bit branch masks held as int) --------------- */
function hb_pow2(n : int) -> int = { var r : int = 1; var i : int = 0; while i < n do { r = r * 2; i = i + 1 }; r }
function hb_bit_set(mask : int, i : int) -> bool = emod_int(ediv_int(mask, hb_pow2(i)), 2) == 1
function hb_with_bit(mask : int, i : int) -> int = if hb_bit_set(mask, i) then mask else mask + hb_pow2(i)
function hb_popcount16(mask : int) -> int = {
  var c : int = 0;
  foreach (i from 0 to 15) { if hb_bit_set(mask, i) then c = c + 1 else () };
  c
}

/* mask-vector ops over list(int) (index 0 = depth 0) */
function hb_mlen(ms : list(int)) -> int = list_len(ms)
function hb_mget(ms : list(int), i : int) -> int = list_nth(ms, i, 0)
function hb_mset(ms : list(int), i : int, v : int) -> list(int) = {
  let l = if hb_mlen(ms) > (i + 1) then hb_mlen(ms) else (i + 1);
  var out : list(int) = [||];
  foreach (k from 0 to (l - 1)) { out = (if k == i then v else list_nth(ms, k, 0)) :: out };
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
      (x :: ta, y :: tb) => if x == y then { i = i + 1; ra = ta; rb = tb } else stop = true,
      (_, _) => stop = true
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
function hb_word_ref(hash : list(byte)) -> list(byte) = list_append([| 0xa0 |], hash)

function hb_branch_payload(mask : int, children : list(list(byte))) -> list(byte) = {
  var payload : list(byte) = [||];
  var ch : list(list(byte)) = children;   /* ascending nibble order */
  foreach (i from 0 to 15) {
    if hb_bit_set(mask, i) then {
      match ch { c :: t => { payload = list_append(payload, c); ch = t }, [||] => () }
    } else payload = list_append(payload, [| 0x80 |])
  };
  list_append(payload, [| 0x80 |])        /* branch value slot: empty */
}
function hb_branch_ref(mask : int, children : list(list(byte))) -> list(byte) =
  child_ref(rlp_list(hb_branch_payload(mask, children)))

/* ---- HashBuilder state (registers; single-threaded root computation) ---- */
register hb_key     : list(int)        = [||]   /* current key nibbles            */
register hb_val     : list(byte)       = [||]   /* current value (leaf) or hash    */
register hb_is_hash : bool             = false  /* value is a blinded branch hash  */
register hb_stack   : list(list(byte)) = [||]   /* stack of child-refs; head = top */
register hb_masks   : list(int)        = [||]   /* state masks; index 0 = depth 0  */

function hb_reset() -> unit = { hb_key = [||]; hb_val = [||]; hb_is_hash = false; hb_stack = [||]; hb_masks = [||] }

/* build a branch node at depth `len` from the top popcount(mask) stack items */
function hb_push_branch(len : int) -> unit = {
  let mask = hb_mget(hb_masks, len);
  let k = hb_popcount16(mask);
  let children = list_reverse(hb_ltake(hb_stack, k));   /* head=highest nibble -> ascending */
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
    let preceding_len = if hb_mlen(hb_masks) == 0 then 0 else hb_mlen(hb_masks) - 1;
    let common = hb_cpl(succ, current);
    let len = if preceding_len > common then preceding_len else common;
    assert(len < list_len(current), "hb_update: len < current.len");
    let extra = list_nth(current, len, 0);
    hb_masks = hb_mset(hb_masks, len, hb_with_bit(hb_mget(hb_masks, len), extra));
    let len_from = if (list_len(succ) > 0) | preceding_exists then len + 1 else len;
    let short_key = hb_ldrop(current, len_from);
    if not_bool(build_extensions) then {
      if hb_is_hash then { hb_stack = hb_word_ref(hb_val) :: hb_stack; build_extensions = true }
      else hb_stack = hb_leaf_ref(short_key, hb_val) :: hb_stack
    } else ();
    if build_extensions & (list_len(short_key) > 0) then {
      match hb_stack { last :: rest => hb_stack = hb_ext_ref(short_key, last) :: rest, [||] => () }
    } else ();
    if (preceding_len <= common) & (list_len(succ) > 0) then done = true
    else {
      if (list_len(succ) > 0) | preceding_exists then hb_push_branch(len) else ();
      hb_masks = hb_ltake(hb_masks, len);
      if preceding_len == 0 then done = true
      else {
        current = hb_ltake(current, preceding_len);
        var popping : bool = true;
        while popping do {
          let l = hb_mlen(hb_masks);
          if l > 0 & hb_mget(hb_masks, l - 1) == 0 then hb_masks = hb_ltake(hb_masks, l - 1) else popping = false
        };
        build_extensions = true
      }
    }
  }
}

function hb_add_leaf(key : list(int), value : list(byte)) -> unit = {
  if list_len(hb_key) > 0 then hb_update(key) else ();
  hb_key = key; hb_val = value; hb_is_hash = false
}
function hb_add_branch(key : list(int), hash : list(byte)) -> unit = {
  if list_len(hb_key) > 0 then hb_update(key)
  else if list_len(key) == 0 then hb_stack = hb_word_ref(hash) :: hb_stack else ();
  hb_key = key; hb_val = hash; hb_is_hash = true
}

function hb_current_root() -> bits(256) =
  match hb_stack {
    [||]      => keccak256([| 0x80 |]),   /* empty trie: keccak(rlp("")) */
    top :: _  => if list_len(top) == 33 then hb_bytes_be256(hb_ldrop(top, 1)) else keccak256(top)
  }
function hb_root() -> bits(256) = {
  if list_len(hb_key) > 0 then { hb_update([||]); hb_key = [||]; hb_val = [||] } else ();
  hb_current_root()
}

/* ---- differential self-check: stack builder == recursive trie_root ------ */
/* add the (already key-sorted) pairs via hb_add_leaf, compare to trie_root.  */
function hb_root_of(pairs : list(TriePair)) -> bits(256) = {
  hb_reset();
  var ps : list(TriePair) = pairs;
  while (match ps { [||] => false, _ => true }) do {
    match ps { p :: r => { hb_add_leaf(p.tp_key, p.tp_val); ps = r }, [||] => () }
  };
  hb_root()
}
function hb_tp(key : list(int), value : list(byte)) -> TriePair = struct { tp_key = key, tp_val = value }
function hb_repeat_byte(b : byte, n : int) -> list(byte) = { var out : list(byte) = [||]; var i : int = 0; while i < n do { out = b :: out; i = i + 1 }; out }
function hb_check_set(pairs : list(TriePair)) -> unit =
  assert(hb_root_of(pairs) == trie_root(pairs), "hash_builder root == recursive trie_root")

/* child-ref (hash form) of the subtree that all `group` keys share below the
 * first nibble -- i.e. the value a parent branch holds for that nibble slot.
 * (Built via the recursive oracle at depth 1; used to blind a whole subtree.) */
function hb_subtree_ref(group : list(TriePair)) -> list(byte) = child_ref(build_node(group, 1))

/* Partial-witness check: present some leaves, blind a whole sibling subtree as a
 * single hash via hb_add_branch, and require the re-rooted partial trie to equal
 * the full trie root. `present`/`blinded` partition `full` by first nibble; the
 * blinded group hangs off `blind_nib`. Run in both interleave orders. */
function hb_add_pairs(ps : list(TriePair)) -> unit = {
  var r : list(TriePair) = ps;
  while (match r { [||] => false, _ => true }) do {
    match r { p :: t => { hb_add_leaf(p.tp_key, p.tp_val); r = t }, [||] => () }
  }
}
function hb_blind_set(present : list(TriePair), blinded : list(TriePair),
                      blind_nib : int, blind_first : bool, full : list(TriePair)) -> unit = {
  let sref = hb_subtree_ref(blinded);
  assert(list_len(sref) == 33, "blinded subtree is a hash (>=32B node)");
  let h = hb_ldrop(sref, 1);
  hb_reset();
  if blind_first then { hb_add_branch([| blind_nib |], h); hb_add_pairs(present) }
  else { hb_add_pairs(present); hb_add_branch([| blind_nib |], h) };
  assert(hb_root() == trie_root(full), "blinded partial root == full trie root")
}

function hb_blind_selfcheck() -> unit = {
  let g1 : list(TriePair) = [| hb_tp([| 1, 0, 0, 0 |], [| 0x11 |]),
                               hb_tp([| 1, 0, 0, 1 |], [| 0x12 |]),
                               hb_tp([| 1, 5, 0, 0 |], [| 0x13 |]) |];
  let g7 : list(TriePair) = [| hb_tp([| 7, 2, 0, 0 |], [| 0x71 |]),
                               hb_tp([| 7, 2, 0, 1 |], [| 0x72 |]),
                               hb_tp([| 7, 9, 0, 0 |], [| 0x79 |]) |];
  let full : list(TriePair) = list_append(g1, g7);
  /* present nibble-1 leaves, blind the nibble-7 subtree (branch comes last) */
  hb_blind_set(g1, g7, 7, false, full);
  /* present nibble-7 leaves, blind the nibble-1 subtree (branch comes first) */
  hb_blind_set(g7, g1, 1, true, full)
}

function hb_selfcheck() -> unit = {
  /* single leaf */
  hb_check_set([| hb_tp([| 1, 2, 3, 4 |], [| 0xaa |]) |]);
  /* two leaves sharing a prefix -> extension over a branch */
  hb_check_set([| hb_tp([| 1, 2, 3, 4 |], [| 0xaa |]), hb_tp([| 1, 2, 5, 6 |], [| 0xbb |]) |]);
  /* immediate branch at root (distinct first nibble) */
  hb_check_set([| hb_tp([| 0, 1, 2, 3 |], [| 0x01 |]), hb_tp([| 7, 1, 2, 3 |], [| 0x07 |]),
                  hb_tp([| 9, 9, 9, 9 |], [| 0x09 |]) |]);
  /* longer secure-trie-like keys (8 nibbles), several branch levels */
  hb_check_set([| hb_tp([| 10, 11, 0, 0, 0, 0, 0, 1 |], [| 0x31 |]),
                  hb_tp([| 10, 11, 0, 0, 0, 0, 0, 2 |], [| 0x32 |]),
                  hb_tp([| 10, 12, 0, 0, 0, 0, 0, 0 |], [| 0x33 |]),
                  hb_tp([| 15, 0,  1, 2, 3, 4, 5, 6 |], [| 0x34 |]) |]);
  /* value >= 32 bytes (child hashed, not inline) */
  hb_check_set([| hb_tp([| 2, 2, 2, 2 |], hb_repeat_byte(0xcd, 34)),
                  hb_tp([| 2, 2, 9, 9 |], [| 0x77 |]) |])
}
