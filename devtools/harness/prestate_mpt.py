#!/usr/bin/env python3
"""Build a secure-MPT from an EEST alloc via execution-specs, collect all
trie nodes (hash -> rlp), and return the state root. Verified by building a
fixture's POST alloc and matching its expected postStateHash."""

import json
import os
import sys
from typing import Any

from devtools.paths import REPO_ROOT

# execution-specs is expected beside this repository; override for other layouts.
_ELDIR = str(REPO_ROOT)
_EXECSPECS_ROOT = os.environ.get(
    "EXECSPECS_ROOT", os.path.abspath(os.path.join(_ELDIR, "..", "execution-specs"))
)
sys.path.insert(0, os.path.join(_EXECSPECS_ROOT, "src"))

import ethereum.merkle_patricia_trie as M
from ethereum.crypto.hash import keccak256
from ethereum.merkle_patricia_trie import (
    BranchNode,
    ExtensionNode,
    LeafNode,
    Trie,
    nibble_list_to_compact,
    root,
    trie_set,
)
from ethereum.state import Account
from ethereum_rlp import rlp
from ethereum_types.bytes import Bytes
from ethereum_types.numeric import U256, Uint

NODES = {}  # keccak(rlp) -> rlp bytes, for every internal node >= 32 bytes


def _encode_internal_node(node):
    unencoded: Any
    if node is None:
        unencoded = b""
    elif isinstance(node, LeafNode):
        unencoded = (nibble_list_to_compact(node.rest_of_key, True), node.value)
    elif isinstance(node, ExtensionNode):
        unencoded = (nibble_list_to_compact(node.key_segment, False), node.subnode)
    elif isinstance(node, BranchNode):
        unencoded = [*list(node.subnodes), node.value]
    else:
        raise AssertionError(type(node))
    encoded = rlp.encode(unencoded)
    if len(encoded) < 32:
        return unencoded
    h = keccak256(encoded)
    NODES[bytes(h)] = bytes(encoded)
    return h


M.encode_internal_node = _encode_internal_node  # patch (patricialize calls it by module name)


def _u256(x):
    return U256(int(x, 16) if isinstance(x, str) else int(x))


def build(alloc):
    """alloc: {addr_hex: {nonce,balance,code,storage:{k:v}}} -> (state_root_hex, NODES)."""
    NODES.clear()
    account_trie = Trie(secured=True, default=None)
    storage_roots = {}
    for addr_hex, acc in alloc.items():
        addr = Bytes(bytes.fromhex(addr_hex[2:].rjust(40, "0")))
        code = bytes.fromhex(acc.get("code", "0x")[2:])
        st = Trie(secured=True, default=U256(0))
        for k, v in acc.get("storage", {}).items():
            if int(v, 16) != 0:
                trie_set(st, Bytes(int(k, 16).to_bytes(32, "big")), _u256(v))
        sroot = root(st)
        storage_roots[addr] = sroot
        account = Account(
            nonce=Uint(int(acc.get("nonce", "0x0"), 16)),
            balance=_u256(acc.get("balance", "0x0")),
            code_hash=keccak256(code),
        )
        trie_set(account_trie, addr, account)
    sr = root(account_trie, lambda a: storage_roots[a])
    return "0x" + bytes(sr).hex(), dict(NODES)


def serve():
    """Persistent builder: one JSON alloc per stdin line -> one JSON result line
    {"root": "0x..", "nodes": ["<hex rlp>", ...]}. Lets the harness CLI (which lacks
    the execution-specs deps) build pre-state tries via this interpreter."""
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        alloc = json.loads(line)
        sr, nodes = build(alloc)
        sys.stdout.write(
            json.dumps({"root": sr, "nodes": [v.hex() for v in nodes.values()]}) + "\n"
        )
        sys.stdout.flush()


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--serve":
        serve()
    else:
        with open(sys.argv[1]) as fixture_file:
            fx = json.load(fixture_file)
        _, case = next(iter(fx.items()))
        sr, nodes = build(case["pre"])
        print("pre-state root:", sr, "nodes:", len(nodes))
