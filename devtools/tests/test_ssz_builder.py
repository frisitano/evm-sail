"""Signing-domain regressions at the state-test adapter boundary."""

import unittest

from devtools.harness.ssz_builder import _chain_id, _transaction


class SigningDomainTests(unittest.TestCase):
    def test_environment_chain_id_is_preserved(self) -> None:
        case = {
            "env": {"currentChainId": "0x2"},
            "config": {},
            "tx": {
                "nonce": "0x0",
                "gasLimit": ["0x10000"],
                "data": ["0x"],
                "value": ["0x0"],
                "secretKey": "0x01",
                "gasPrice": "0x7",
            },
            "idx": {"gas": 0, "data": 0, "value": 0},
        }
        chain_id = _chain_id(case)
        self.assertEqual(chain_id, 2)
        self.assertEqual(_transaction(case, chain_id)["chainId"], 2)

    def test_explicit_chain_config_overrides_environment(self) -> None:
        self.assertEqual(
            _chain_id({"config": {"chainid": "0x3"}, "env": {"currentChainId": "0x2"}}),
            3,
        )

    def test_default_chain_id(self) -> None:
        self.assertEqual(_chain_id({"env": {}}), 1)
