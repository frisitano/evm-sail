"""Regression checks for complete fork selection and pinned EELS shards."""

import json
import tempfile
import unittest
from pathlib import Path
from typing import Any, cast

from devtools.harness.cli import collect, fixture_items


class FixtureDiscoveryTests(unittest.TestCase):
    def test_no_fork_filter_collects_historical_entries(self) -> None:
        fixture = {
            "case": {
                "env": {},
                "pre": {},
                "transaction": {"gasLimit": ["0x5208"], "value": ["0x00"], "data": ["0x"]},
                "post": {
                    fork: [{"indexes": {"gas": 0, "data": 0, "value": 0}}]
                    for fork in ("Berlin", "London", "Amsterdam")
                },
            }
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixtures.json"
            path.write_text(json.dumps(fixture))
            self.assertEqual(
                [case["fork"] for case in cast(Any, collect)(str(path), None, 0)],
                ["Berlin", "London", "Amsterdam"],
            )
            self.assertEqual(len(cast(Any, collect)(str(path), "London", 0)), 1)

    def test_inline_and_part_shards(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "fixture.part").write_text(json.dumps({"env": {"number": 1}}))
            shard = root / "fixtures.jsonl"
            shard.write_text(
                json.dumps({"k": "inline", "v": {"env": {"number": 0}}})
                + "\n"
                + json.dumps({"k": "part", "p": "fixture.part"})
                + "\n"
            )
            self.assertEqual(
                list(cast(Any, fixture_items)(str(shard))),
                [("inline", {"env": {"number": 0}}), ("part", {"env": {"number": 1}})],
            )

    def test_shard_part_cannot_escape_its_directory(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixtures.jsonl"
            path.write_text(json.dumps({"k": "case", "p": "../outside.part"}) + "\n")
            with self.assertRaisesRegex(ValueError, "inside its shard directory"):
                list(cast(Any, fixture_items)(str(path)))
