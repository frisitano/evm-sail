from __future__ import annotations

import json
import os
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from tools.benchmark_zisk import (
    FixtureCase,
    Guest,
    export_dashboard_data,
    frame_guest_input,
    fixture_taxonomy,
    parse_comprehensive_opcode_costs,
    parse_cost,
    parse_executed_functions,
    parse_opcode_costs,
    parse_profile_scope_costs,
    parse_profile_scope_steps,
    parse_steps,
    parse_top_cost_functions,
)


REPO_ROOT = Path(__file__).resolve().parents[1]
TMP_ROOT = Path(os.environ.get("AGENT_TMPDIR", REPO_ROOT / ".agent-tmp"))


class ProfileParserTests(unittest.TestCase):
    def test_only_evm_sail_receives_private_capacity_metadata(self) -> None:
        evm_sail = Guest("evm-sail", Path("evm-sail.elf"))
        reth = Guest("reth", Path("reth.elf"))
        with patch(
            "harness.dump_state.prepare_optimized_input",
            return_value=b"payload-with-capacities",
        ) as prepare:
            framed_evm_sail = frame_guest_input(evm_sail, b"payload")
            framed_reth = frame_guest_input(reth, b"payload")

        prepare.assert_called_once_with(b"payload")
        self.assertEqual(
            framed_evm_sail[:8],
            len(b"payload-with-capacities").to_bytes(8, "little"),
        )
        self.assertEqual(
            framed_evm_sail[8 : 8 + len(b"payload-with-capacities")],
            b"payload-with-capacities",
        )
        self.assertEqual(framed_reth[:8], len(b"payload").to_bytes(8, "little"))
        self.assertEqual(framed_reth[8 : 8 + len(b"payload")], b"payload")

    def test_parses_profile_tag_rows_only(self) -> None:
        report = """
║  ◆ STEPS PROFILE TAGS                                      ║
╠═════════════════════════════════════════════════════════════╣
║  execute_block     ███████████████░░░░     420,224  49.6%  ║
║  serialize_output  ░░░░░░░░░░░░░░░░░░░       1,572   0.2%  ║
╚═════════════════════════════════════════════════════════════╝
║  ◆ COST PROFILE TAGS                                       ║
╠═════════════════════════════════════════════════════════════╣
║  execute_block     ███████████████░░░░  210,112,000  55.1%  ║
║  serialize_output  ░░░░░░░░░░░░░░░░░░░      786,000   0.2%  ║
╚═════════════════════════════════════════════════════════════╝
"""
        self.assertEqual(
            parse_profile_scope_steps(report),
            {"execute_block": 420224, "serialize_output": 1572},
        )
        self.assertEqual(
            parse_profile_scope_costs(report),
            {"execute_block": 210112000, "serialize_output": 786000},
        )

    def test_rejects_report_without_profile_tags(self) -> None:
        with self.assertRaisesRegex(RuntimeError, "profile-tag section"):
            parse_profile_scope_steps("STEPS: 42")

    def test_parses_boxed_sdk_total_steps(self) -> None:
        self.assertEqual(parse_steps("║  STEPS     846,724  ║"), 846724)
        self.assertEqual(parse_cost("║  COST  381,357,833  ║"), 381357833)
        self.assertEqual(parse_steps("STEPS     846,724"), 846724)
        self.assertEqual(parse_cost("TOTAL 381,357,833 100.00%"), 381357833)

    def test_parses_sdk_top_cost_functions(self) -> None:
        report = """
║  ◆ TOP COST FUNCTIONS                                      ║
╠═════════════════════════════════════════════════════════════╣
║   0 evm_execute                  ███████░░░    12,345  31.2% ║
║   1 sha256                       ██░░░░░░░░     2,000   5.1% ║
╚═════════════════════════════════════════════════════════════╝
"""
        self.assertEqual(
            parse_top_cost_functions(report),
            [
                {
                    "name": "evm_execute",
                    "cost": 12345,
                    "share_percent": 31.2,
                },
                {"name": "sha256", "cost": 2000, "share_percent": 5.1},
            ],
        )

    def test_parses_sdk_opcode_costs(self) -> None:
        report = """
║  ◆ COST DISTRIBUTION BY OPCODE                  ║  ◆ OPS vs FROPS    ║
╠══════════════════════════════════════════════════════════════════════╣
║  OPCODE                              COST      %  ║  OPS FROPS       ║
║  keccak       ███░░░░░░░░░░░░░     9,670,400   2.8%  ║             ║
║  signextend_w ░░░░░░░░░░░░░░░       762,882   0.2%  ║  762,882 0  ║
║  Total                            10,433,282   3.0%  ║             ║
╚══════════════════════════════════════════════════════════════════════╝
"""
        self.assertEqual(
            parse_opcode_costs(report),
            [
                {
                    "name": "keccak",
                    "cost": 9670400,
                    "share_percent": 2.8,
                },
                {
                    "name": "signextend_w",
                    "cost": 762882,
                    "share_percent": 0.2,
                },
            ],
        )

    def test_parses_all_used_operations_from_full_report(self) -> None:
        report = """
OP add 1,000 10.00% 2,000 20.00%
OP keccak 2 0.02% 7,000 70.00%
FROP add 200 2.00% 1,000 10.00%
TOTAL 10,000 100.00%
"""
        self.assertEqual(
            parse_comprehensive_opcode_costs(report),
            [
                {
                    "name": "keccak",
                    "op_count": 2,
                    "frop_count": 0,
                    "op_cost": 7000,
                    "frop_cost": 0,
                    "cost": 7000,
                    "share_percent": 70.0,
                },
                {
                    "name": "add",
                    "op_count": 1000,
                    "frop_count": 200,
                    "op_cost": 2000,
                    "frop_cost": 1000,
                    "cost": 3000,
                    "share_percent": 30.0,
                },
            ],
        )

    def test_aggregates_exclusive_steps_by_non_local_symbol(self) -> None:
        disassembly = """
00000000 <_start>:
       0: 10 add x0,x0,x0
00000004 <.Lloop>:
       4: 20 add x0,x0,x0
00000008 <main>:
       8: 30 add x0,x0,x0
"""
        self.assertEqual(
            parse_executed_functions(disassembly, 60),
            [
                {
                    "name": "_start",
                    "exclusive_steps": 30,
                    "share_percent": 50.0,
                },
                {
                    "name": "main",
                    "exclusive_steps": 30,
                    "share_percent": 50.0,
                },
            ],
        )


class DashboardExportTests(unittest.TestCase):
    def test_fixture_taxonomy_uses_retained_corpus_path(self) -> None:
        taxonomy = fixture_taxonomy(
            Path(
                "/work/current-v062-full/blockchain_tests/for_amsterdam/"
                "shanghai/eip3855_push0/push0/contracts.json"
            )
        )
        self.assertEqual(taxonomy["corpus"], "current-v062-full")
        self.assertEqual(taxonomy["suite"], "blockchain_tests")
        self.assertEqual(taxonomy["target"], "for_amsterdam")
        self.assertEqual(taxonomy["fork"], "shanghai")
        self.assertEqual(taxonomy["feature"], "eip3855_push0")
        self.assertEqual(taxonomy["category"], "push0")

    def test_export_writes_catalog_and_lazy_fixture_shard(self) -> None:
        TMP_ROOT.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as raw_tmp:
            tmp = Path(raw_tmp)
            fixture = (
                tmp
                / "current-v062-full"
                / "blockchain_tests"
                / "for_amsterdam"
                / "shanghai"
                / "eip3855_push0"
                / "push0.json"
            )
            case = FixtureCase(
                fixture=fixture,
                fixture_sha256="ab" * 32,
                name="push0 works",
                block_index=0,
                payload=b"input",
                expected=b"output",
            )
            elf = tmp / "guest.elf"
            elf.write_bytes(b"elf")
            guests = [Guest("evm-sail", elf), Guest("reth", elf)]
            guest_results = {}
            for guest in guests:
                instrumented = guest.name == "evm-sail"
                guest_results[guest.name] = {
                    "elf": {
                        "path": str(elf),
                        "size_bytes": 3,
                        "sha256": "cd" * 32,
                    },
                    "cases": [
                        {
                            "measurements": [
                                {"steps": 1000, "elapsed_ns": 1}
                            ]
                        }
                    ],
                    "profiles": [
                        {
                            "artifacts": {
                                "steps": 1004 if instrumented else 800,
                                "cost": 4000 if instrumented else 3200,
                                "scope_steps": (
                                    {
                                        "stateless_validation": 900,
                                        "execute_block": 500,
                                    }
                                    if instrumented
                                    else {}
                                ),
                                "scope_costs": (
                                    {
                                        "stateless_validation": 3600,
                                        "execute_block": 2000,
                                    }
                                    if instrumented
                                    else {}
                                ),
                                "top_cost_functions": [
                                    {
                                        "name": "sha256",
                                        "cost": 1200,
                                        "share_percent": 30.0,
                                    }
                                ],
                                "opcode_costs": [
                                    {
                                        "name": "keccak",
                                        "cost": 1500 if instrumented else 1000,
                                        "op_count": 20,
                                        "frop_count": 0,
                                        "op_cost": (
                                            1500 if instrumented else 1000
                                        ),
                                        "frop_cost": 0,
                                        "share_percent": (
                                            37.5 if instrumented else 31.25
                                        ),
                                    }
                                ],
                                "opcode_profile_completeness": (
                                    "all_used_operations"
                                ),
                                "function_profile_status": (
                                    "ok"
                                    if instrumented
                                    else "stack_mismatch"
                                ),
                                "executed_functions": [
                                    {
                                        "name": "main",
                                        "exclusive_steps": (
                                            1004 if instrumented else 800
                                        ),
                                        "share_percent": 100.0,
                                    }
                                ],
                                "function_inventory_status": "ok",
                            }
                        }
                    ],
                }
            benchmark_result = {
                "generated_at_unix_ns": 123,
                "ziskemu": {"version": "ziskemu 0.15.0"},
                "metadata": {"commit": "deadbeef"},
            }
            output = tmp / "dashboard"

            export_dashboard_data(
                output, [case], guests, guest_results, benchmark_result
            )

            catalog = json.loads((output / "catalog.json").read_text())
            self.assertEqual(catalog["case_count"], 1)
            self.assertEqual(catalog["fixture_count"], 1)
            self.assertEqual(
                [guest["name"] for guest in catalog["guests"]],
                ["evm-sail", "reth"],
            )
            self.assertEqual(catalog["generated_at_unix_ns"], "123")
            fixture_entry = catalog["fixtures"][0]
            shard = json.loads((output / fixture_entry["shard"]).read_text())
            self.assertEqual(shard["format_version"], 4)
            case_guests = shard["cases"][0]["guests"]
            self.assertEqual(case_guests["evm-sail"]["total_steps"], 1004)
            self.assertEqual(case_guests["evm-sail"]["total_cost"], 4000)
            self.assertEqual(
                case_guests["evm-sail"]["scope_steps"]["execute_block"], 500
            )
            self.assertEqual(
                case_guests["evm-sail"]["scope_costs"]["execute_block"], 2000
            )
            self.assertEqual(case_guests["reth"]["scope_costs"], {})
            self.assertEqual(
                case_guests["reth"]["top_cost_functions"][0]["name"], "sha256"
            )
            self.assertEqual(
                case_guests["evm-sail"]["opcode_costs"][0]["name"], "keccak"
            )
            self.assertEqual(
                case_guests["reth"]["function_profile_status"],
                "stack_mismatch",
            )
            self.assertEqual(
                case_guests["reth"]["executed_functions"][0][
                    "exclusive_steps"
                ],
                800,
            )
            self.assertEqual(
                case_guests["evm-sail"]["opcode_profile_completeness"],
                "all_used_operations",
            )


if __name__ == "__main__":
    unittest.main()
