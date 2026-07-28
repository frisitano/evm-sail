"""Unit tests for the Sail mkdocstrings handler (no MkDocs build needed).

Run from the mkdocstrings-sail directory with any interpreter that has the
package's dependencies, e.g.:

    uv run --with . python -m unittest discover tests
"""

from __future__ import annotations

import unittest

from mkdocstrings_handlers.sail import Bundle, BundleError, anchor_for
from mkdocstrings_handlers.sail._book import markdown_blocks, page_items, render_page
from mkdocstrings_handlers.sail._handler import (
    _dedent_comment,
    _body_preview,
    _byte_to_str_spans,
    _comment_summary,
    _highlight_linked,
)
from mkdocstrings_handlers.sail._lsp import _LineIndex
from mkdocstrings_handlers.sail._index import LspIndex
from mkdocstrings_handlers.sail._lsp import decode_semantic_tokens

# Shaped exactly like sail --doc --doc-format identity --doc-embed plain
# --doc-embed-with-location output (loc = [line1, bol1, char1, line2, bol2, char2]).
BUNDLE = {
    "version": 1,
    "embedding": "plain",
    "functions": {
        "step": {
            "function": {
                "number": 0,
                "source": {
                    "contents": "function step() = {\n    PC = increment(PC)\n}",
                    "file": "spec/core/machine.sail",
                    "loc": [7, 122, 122, 9, 165, 166],
                },
                "comment": " Advance the machine. ",
            },
            "links": [
                {"type": "register", "id": "PC", "file": "spec/core/machine.sail", "loc": [146, 148]},
                {"type": "function", "id": "increment", "file": "spec/core/machine.sail", "loc": [151, 160]},
                {"type": "register", "id": "PC", "file": "spec/core/machine.sail", "loc": [161, 163]},
            ],
        },
        "increment": {
            "function": {
                "number": 0,
                "source": "function increment(w) = w",
            }
        },
    },
    "vals": {
        "increment": {
            "val": {
                "source": {
                    "contents": "val increment : word -> word",
                    "file": "spec/lib/util.sail",
                    "loc": [9, 129, 129, 9, 129, 157],
                },
                "comment": " Increment a word. ",
            }
        }
    },
    "types": {
        "word": {
            "type": {
                "contents": "type word = bits(8)",
                "file": "spec/lib/util.sail",
                "loc": [6, 74, 74, 6, 74, 93],
            },
            "comment": " The machine word type. ",
        }
    },
    "mappings": {
        "flag_bit": [
            {
                "number": 0,
                "source": "flag_bit : bool <-> bits(1) = {true <-> 0b1, false <-> 0b0}",
            }
        ]
    },
    "lets": {
        # a function-typed link whose id is only a mapping must retarget
        "uses_mapping": {
            "let": {
                "source": {
                    "contents": "let x = flag_bit(true)",
                    "file": "spec/core/machine.sail",
                    "loc": [12, 200, 200, 12, 200, 222],
                }
            },
            "links": [
                {"type": "function", "id": "flag_bit", "file": "spec/core/machine.sail", "loc": [208, 216]},
            ],
        }
    },
}


class TestBundle(unittest.TestCase):
    def setUp(self):
        self.bundle = Bundle(dict(BUNDLE))

    def test_anchor_scheme_matches_mkdocs_target(self):
        self.assertEqual(anchor_for("function", "step"), "function-step")
        self.assertEqual(anchor_for("function", "operator +"), "function-operator--")

    def test_find_prefers_function_over_val(self):
        self.assertEqual(self.bundle.find("increment").kind, "function")
        self.assertEqual(self.bundle.find("increment", kind="val").kind, "val")

    def test_find_unknown_raises(self):
        with self.assertRaises(BundleError):
            self.bundle.find("nonexistent")

    def test_version_check(self):
        with self.assertRaises(BundleError):
            Bundle({"version": 2})

    def test_comment_and_clauses(self):
        step = self.bundle.find("step")
        self.assertEqual(step.comment, " Advance the machine. ")
        self.assertEqual(len(step.clauses), 1)
        self.assertTrue(step.clauses[0].text.startswith("function step()"))

    def test_link_offsets_are_relative_and_in_range(self):
        step = self.bundle.find("step")
        clause = step.clauses[0]
        links = clause.links_within(step.links)
        self.assertEqual(len(links), 3)
        for start, end, anchor in links:
            self.assertEqual(clause.text[start:end], {"register-PC": "PC", "function-increment": "increment"}[anchor])

    def test_type_comment_from_sibling_key(self):
        word = self.bundle.find("word")
        self.assertEqual(word.comment, " The machine word type. ")

    def test_mapping_application_link_retargets(self):
        uses = self.bundle.find("uses_mapping")
        self.assertEqual([l.kind for l in uses.links], ["mapping"])
        self.assertEqual(uses.links[0].anchor, "mapping-flag_bit")
        clause = uses.clauses[0]
        (start, end, anchor) = clause.links_within(uses.links)[0]
        self.assertEqual(clause.text[start:end], "flag_bit")

    def test_raw_source_has_no_links(self):
        increment = self.bundle.find("increment")
        clause = increment.clauses[0]
        self.assertIsNone(clause.start)
        self.assertEqual(clause.links_within(increment.links), [])


class TestHighlightLinked(unittest.TestCase):
    def test_autorefs_wrap_exact_spans(self):
        step = Bundle(dict(BUNDLE)).find("step")
        clause = step.clauses[0]
        html = _highlight_linked(clause.text, clause.links_within(step.links))
        self.assertIn('<autoref identifier="function-increment" optional>', html)
        self.assertEqual(html.count('<autoref identifier="register-PC" optional>'), 2)
        self.assertEqual(html.count("</autoref>"), 3)
        # keyword highlighting from the Sail lexer survives alongside the links
        self.assertIn('<span class="k">function</span>', html)

    def test_html_is_escaped(self):
        html = _highlight_linked('let s = "a <b> & c"', [])
        self.assertNotIn("<b>", html)
        self.assertIn("&lt;b&gt;", html)

    def test_explicit_semantic_tokens_override_lexer(self):
        html = _highlight_linked("foo bar", [(4, 7, "type-bar")], tokens=[(0, 3, "nf"), (4, 7, "kt")])
        self.assertIn('<span class="nf">foo</span>', html)
        self.assertIn('<autoref identifier="type-bar" optional><span class="kt">bar</span></autoref>', html)

    def test_hover_card_key_becomes_data_attribute(self):
        html = _highlight_linked("foo bar", [(4, 7, "type-bar", None, "type-bar")], tokens=[])
        self.assertIn('<span data-sail-hover="type-bar">', html)
        self.assertNotIn("title=", html)

    def test_link_tooltip_becomes_title_span(self):
        html = _highlight_linked("foo bar", [(4, 7, "type-bar", 'bits(8) — A "word".')], tokens=[])
        self.assertIn(
            '<autoref identifier="type-bar" optional><span title="bits(8) — A &#34;word&#34;.">bar</span></autoref>',
            html,
        )

    def test_body_preview_truncates(self):
        step = Bundle(dict(BUNDLE)).find("step")
        preview = _body_preview(step)
        self.assertTrue(preview.startswith("function step()"))
        long = Bundle(
            {
                "version": 1,
                "functions": {"f": {"function": {"source": "function f() = {\n" + "    x;\n" * 20 + "}"}}},
            }
        ).find("f")
        preview = _body_preview(long)
        self.assertEqual(len(preview.splitlines()), 9)  # 8 lines + ellipsis
        self.assertTrue(preview.endswith("…"))
        self.assertIsNone(_body_preview(None))

    def test_dedent_comment_prevents_code_blocks(self):
        comment = " First line.\n\n    Second paragraph, indented under the comment\n    marker.\n\n    - a list item\n        - nested"
        dedented = _dedent_comment(comment)
        self.assertTrue(dedented.startswith("First line.\n\nSecond paragraph"))
        self.assertIn("\n- a list item\n    - nested", dedented)

    def test_comment_summary_first_sentence_plain_text(self):
        self.assertEqual(
            _comment_summary(" Increment a `word` by one. Wraps on overflow. "),
            "Increment a word by one.",
        )
        self.assertIsNone(_comment_summary(None))
        self.assertIsNone(_comment_summary("   "))


# 'legend' matching sail-lsp indices; tokens/references use absolute offsets
INDEX = {
    "version": 1,
    "legend": ["type", "function", "keyword"],
    "files": {
        "spec/core/machine.sail": {
            # covering "register PC : word" starting at offset 28
            "tokens": [[28, 36, 2], [37, 39, 1], [42, 46, 0]],
        }
    },
    "references": [
        {
            "name": "word",
            "kind": "type",
            "file": "spec/core/machine.sail",
            "start": 42,
            "end": 46,
            "targetFile": "spec/lib/util.sail",
            "targetStart": 74,
        },
        {
            "name": "ADD",
            "kind": "constructor",
            "file": "spec/core/machine.sail",
            "start": 150,
            "end": 153,
            "targetFile": "spec/lib/util.sail",
            "targetStart": 80,
        },
    ],
    "signatures": {"val:increment": "word -> word", "type:word": "bits(8)"},
}


class TestLspIndex(unittest.TestCase):
    def setUp(self):
        self.index = LspIndex(dict(INDEX))

    def test_decode_semantic_tokens(self):
        text = "function step() = {\n    PC = increment(PC)\n}"
        # [deltaLine, deltaStartChar, length, type, modifiers]
        tokens = decode_semantic_tokens([0, 0, 8, 2, 0, 1, 4, 2, 1, 0], text)
        self.assertEqual(tokens, [[0, 8, 2], [24, 26, 1]])
        self.assertEqual(text[24:26], "PC")

    def test_tokens_within_clips_and_maps_classes(self):
        tokens = self.index.tokens_within("spec/core/machine.sail", 28, 46)
        self.assertEqual(tokens, [(0, 8, "k"), (9, 11, "nf"), (14, 18, "kt")])

    def test_references_within_are_clause_relative(self):
        refs = self.index.references_within("spec/core/machine.sail", 28, 46)
        self.assertEqual(len(refs), 1)
        self.assertEqual((refs[0].start, refs[0].end, refs[0].name), (14, 18, "word"))

    def test_signature_falls_back_to_val_entry(self):
        self.assertEqual(self.index.signature("type", "word"), "bits(8)")
        self.assertEqual(self.index.signature("function", "increment"), "word -> word")
        self.assertIsNone(self.index.signature("function", "unknown"))

    def test_lsp_positions_map_to_byte_offsets(self):
        # an em dash (3 bytes, 1 codepoint, 1 utf-16 unit) before the code
        text = "/*md — dash */\nregister PC : word\n"
        index = _LineIndex(text)
        offset = index.offset(1, 9)  # start of "PC"
        self.assertEqual(text.encode("utf-8")[offset : offset + 2], b"PC")

    def test_byte_spans_convert_to_str_indices(self):
        text = "/* — */ PC"
        (span,) = _byte_to_str_spans(text, [(10, 12, "register-PC")])
        self.assertEqual(text[span[0] : span[1]], "PC")
        self.assertEqual(span[2], "register-PC")
        # ascii fast path is identity
        self.assertEqual(_byte_to_str_spans("PC = 1", [(0, 2, "x")]), [(0, 2, "x")])

    def test_definition_at_finds_containing_type(self):
        bundle = Bundle(dict(BUNDLE))
        owner = bundle.definition_at("spec/lib/util.sail", 80)
        self.assertIsNotNone(owner)
        self.assertEqual(owner.anchor, "type-word")
        self.assertIsNone(bundle.definition_at("spec/lib/util.sail", 5000))


class TestEips(unittest.TestCase):
    def test_link_eip_references_wraps_and_records(self):
        from mkdocstrings_handlers.sail._eips import link_eip_references
        seen = set()
        out = link_eip_references("Per EIP-2929, cold access. See `EIP-9999` in code.", seen, hover=True)
        self.assertIn("[EIP-2929](https://eips.ethereum.org/EIPS/eip-2929)", out)
        self.assertIn('data-sail-hover="eip-2929"', out)
        self.assertIn("`EIP-9999`", out)  # code spans untouched
        self.assertEqual(seen, {2929})

    def test_existing_links_not_rewrapped(self):
        from mkdocstrings_handlers.sail._eips import link_eip_references
        seen = set()
        out = link_eip_references("[EIP-140](https://example.com)", seen, hover=True)
        self.assertEqual(out, "[EIP-140](https://example.com)")
        self.assertEqual(seen, set())


SOURCE = """/*md
# Machine

The machine module.
*/

/*! The program counter. */
register PC : word

/*md
## Stepping /* nested comment */ works
*/

val step : unit -> unit

function step() = ()
"""


class TestBook(unittest.TestCase):
    def test_book_plugin_links_authored_markdown_eips(self):
        from mkdocstrings_handlers.sail._plugin import SailBookPlugin

        out = SailBookPlugin().on_page_markdown(
            "Warming follows EIP-2929.",
            page=None,
            config=None,
            files=None,
        )
        self.assertIn("[EIP-2929](https://eips.ethereum.org/EIPS/eip-2929)", out)

    def test_generated_manifest_preserves_authored_reference_pages(self):
        from pathlib import Path as P
        import tempfile
        from mkdocstrings_handlers.sail._book import GeneratedFiles

        with tempfile.TemporaryDirectory() as directory:
            book = P(directory)
            authored = book / "docs/reference/sail/lib/index.md"
            authored.parent.mkdir(parents=True)
            authored.write_text("# Authored\n")

            generated_page = book / "docs/reference/sail/lib/rlp.md"
            generated = GeneratedFiles(book)
            generated.write_text(generated_page, "# Generated\n")
            self.assertTrue(generated_page.exists())

            GeneratedFiles(book)
            self.assertFalse(generated_page.exists())
            self.assertEqual(authored.read_text(), "# Authored\n")

    def test_generated_writer_refuses_authored_collision(self):
        from pathlib import Path as P
        import tempfile
        from mkdocstrings_handlers.sail._book import GeneratedFiles

        with tempfile.TemporaryDirectory() as directory:
            book = P(directory)
            authored = book / "docs/reference/sail/lib/index.md"
            authored.parent.mkdir(parents=True)
            authored.write_text("# Authored\n")

            with self.assertRaises(FileExistsError):
                GeneratedFiles(book).write_text(authored, "# Generated\n")

    def test_lean_page_items_split(self):
        from mkdocstrings_handlers.sail._book import lean_page_items, render_lean_page
        text = "import Sail\n\n/-! # Gas\n\nProse here. -/\n\n/-- The base fee. -/\ndef base_fee : Int := 7\n"
        items = lean_page_items(text)
        self.assertEqual([k for k, _ in items], ["code", "md", "md", "code"])
        self.assertIn("# Gas", items[1][1])
        title, markdown = render_lean_page("Gas", text)
        self.assertEqual(title, "Gas")
        self.assertTrue(markdown.startswith("# Gas"))  # title heading hoisted above the prelude
        self.assertIn("```lean4\ndef base_fee : Int := 7\n```", markdown)
        self.assertIn("The base fee.", markdown)

    def test_lean_linking_and_cards(self):
        from pathlib import Path as P
        import tempfile
        from mkdocstrings_handlers.sail._book import (
            lean_definition_index, lean_page_url, render_lean_page,
        )
        text = (
            "/-! # Gas -/\n\n/-- The base fee. -/\ndef base_fee : Int := 7\n\n"
            "def double_fee : Int := base_fee + base_fee\n"
        )
        with tempfile.TemporaryDirectory() as d:
            root = P(d)
            (root / "Gas.lean").write_text(text)
            files = [root / "Gas.lean"]
            index = lean_definition_index(files, root)
            self.assertEqual(index["base_fee"]["doc"], "The base fee.")
            self.assertEqual(index["base_fee"]["anchor"], "lean-base_fee")
            _, md = render_lean_page("Gas", text, index, lean_page_url(P("Gas.lean")))
        self.assertIn('<span id="lean-base_fee">', md)
        self.assertIn('href="#lean-base_fee" data-sail-hover="lean-base_fee"', md)
        self.assertNotIn("<template", md)  # cards are fetch-on-demand fragments
        from mkdocstrings_handlers.sail._book import lean_card_html
        card = lean_card_html(index["base_fee"])
        self.assertIn("The base fee.", card)
        self.assertIn("sail-hovercard-code", card)

    def test_markdown_blocks_positions_and_nesting(self):
        blocks = markdown_blocks(SOURCE)
        self.assertEqual(len(blocks), 2)
        self.assertTrue(blocks[0][1].startswith("# Machine"))
        self.assertIn("/* nested comment */ works", blocks[1][1])
        self.assertNotIn("*/", blocks[1][1].split("works")[1])

    def test_marker_requires_whitespace(self):
        self.assertEqual(markdown_blocks("/*mdx not a block */"), [])

    def test_page_interleaves_prose_and_definitions(self):
        # register at SOURCE offset of "register PC", function after section block
        reg_at = SOURCE.index("register PC")
        fn_at = SOURCE.index("function step")
        bundle = Bundle(
            {
                "version": 1,
                "embedding": "plain",
                "registers": {
                    "PC": {
                        "register": {
                            "source": {"contents": "register PC : word", "file": "m.sail", "loc": [0, 0, reg_at, 0, 0, reg_at + 18]},
                            "comment": " The program counter. ",
                        }
                    }
                },
                "functions": {
                    "step": {
                        "function": {
                            "source": {"contents": "function step() = ()", "file": "m.sail", "loc": [0, 0, fn_at, 0, 0, fn_at + 20]}
                        }
                    }
                },
            }
        )
        items = page_items(bundle, "m.sail", SOURCE, excluded=set())
        self.assertEqual([item[0] for item in items], ["md", "def", "md", "def"])
        title, markdown = render_page("m.sail", items)
        self.assertEqual(title, "Machine")
        self.assertLess(markdown.index("# Machine"), markdown.index("::: PC"))
        self.assertLess(markdown.index("::: PC"), markdown.index("## Stepping"))
        self.assertLess(markdown.index("## Stepping"), markdown.index("::: step"))
        self.assertIn("kind: register", markdown)


if __name__ == "__main__":
    unittest.main()
