/* ======================================================================== */
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
  while more do { match rest { b :: t => { rlp_scratch = b :: rlp_scratch; rest = t }, [||] => more = false } }
}

/* rlp([nonce, balance, storageRoot, codeHash]) */
function account_rlp(nonce : int, balance : word, sroot : bits(256), chash : bits(256)) -> list(byte) = {
  rlp_scratch = [||];
  rlp_push(rlp_int(nonce));
  rlp_push(rlp_int(unsigned(balance)));
  rlp_push(rlp_word(sroot));
  rlp_push(rlp_word(chash));
  rlp_list(list_reverse(rlp_scratch))   /* prepend builds newest-first; reverse -> field order */
}

/* secure-trie pairs for one account's non-zero storage, enumerated from the
 * flattened C overlay map (cold path: only --root runs / block authentication). */
function storage_pairs(a : address) -> list(TriePair) = {
  var out : list(TriePair) = [||];
  let n = storage_count();
  foreach (j from 0 to (n - 1)) {
    let (ea, es, ev) = storage_entry(j);
    if ea == a & not_bool(word_is_zero(ev))
    then out = (struct {
                  tp_key = to_nibbles(word_to_bytes32(keccak256(word_to_bytes32(es)))),
                  tp_val = rlp_int(unsigned(ev)) }) :: out
    else ()
  };
  out
}
function storage_root(a : address) -> bits(256) = trie_root(storage_pairs(a))

function acc_is_empty(acc : Account) -> bool =
  (acc.acc_codehash == KECCAK_EMPTY) & acc.acc_nonce == 0 & word_is_zero(acc.acc_balance)

/* secure-trie pairs for all non-empty accounts */
function state_pairs(entries : list(AccountEntry)) -> list(TriePair) =
  match entries {
    [||]   => [||],
    e :: r => if acc_is_empty(e.ae_acc) then state_pairs(r)
              else (struct {
                tp_key = to_nibbles(word_to_bytes32(keccak256(address_to_bytes(e.ae_addr)))),
                tp_val = account_rlp(e.ae_acc.acc_nonce, e.ae_acc.acc_balance,
                                     storage_root(e.ae_addr), e.ae_acc.acc_codehash)
              }) :: state_pairs(r)
  }

function compute_state_root() -> bits(256) = trie_root(state_pairs(k_accounts))

/* ---- post-state root via the stack HashBuilder ------------------------- */
/* lexicographic nibble-key order (HashBuilder requires strictly ascending keys) */
function nib_lt(a : list(int), b : list(int)) -> bool = {
  var ra : list(int) = a;
  var rb : list(int) = b;
  var res : int = 0;   /* 0 undecided, 1 a<b, 2 a>=b */
  while res == 0 do {
    match (ra, rb) {
      ([||], [||]) => res = 2,
      ([||], _)    => res = 1,
      (_, [||])    => res = 2,
      (x :: ta, y :: tb) => if x < y then res = 1 else if x > y then res = 2 else { ra = ta; rb = tb }
    }
  };
  res == 1
}
function insert_sorted(p : TriePair, ps : list(TriePair)) -> list(TriePair) =
  match ps {
    [||]   => [| p |],
    q :: r => if nib_lt(p.tp_key, q.tp_key) then p :: ps else q :: insert_sorted(p, r)
  }
function sort_pairs(ps : list(TriePair)) -> list(TriePair) =
  match ps { [||] => [||], q :: r => insert_sorted(q, sort_pairs(r)) }

/* Post-state root via the alloy-trie stack builder: feed the post-execution
 * account leaves (key = keccak(addr), value = rlp(post-account); created accounts
 * included, EIP-161-empty dropped) in sorted key order. The untouched frontier --
 * blinded sibling subtrees for accounts the block never touches -- folds in via
 * hb_add_branch (validated in stage 2); it is empty for a fixture whose whole
 * state is materialised. Same stack machinery as the pre-state validation. */
function compute_state_root_hb() -> bits(256) = hb_root_of(sort_pairs(state_pairs(k_accounts)))

/* Authenticate the current world state against a claimed root. Used both for
 * pre-state authentication (the provided pre-state/witness must hash to the
 * parent block's stateRoot) and post-state authentication (the computed
 * post-state must hash to this block's stateRoot). A stateless prover supplies
 * the accessed pre-state as the world here; verifying individual MPT branch
 * proofs (without the full state) is a further refinement. */
function authenticate_state(claimed_root : bits(256)) -> bool =
  compute_state_root() == claimed_root
