"""mkdocstrings handler for Sail.

Renders definitions from a ``sail --doc`` docinfo bundle: heading with a
stable ``<kind>-<id>`` anchor, doc comment as Markdown, and the definition
source highlighted with the bundled Sail lexer. Function calls and register
uses *inside* the source are wrapped in ``<autoref>`` elements, which
mkdocs-autorefs resolves to wherever those definitions are rendered in the
site — cross-page, with no path computation here.
"""

from __future__ import annotations

import json
import textwrap
from pathlib import Path
from typing import Any, ClassVar, Mapping, Optional

from markupsafe import Markup, escape
from mkdocstrings import BaseHandler, CollectionError, get_logger
from pygments.token import STANDARD_TYPES, Text as TextToken

from ._bundle import Bundle, BundleError, Clause, Definition
from ._eips import link_eip_references
from ._index import IndexReference, LspIndex
from ._lexer import SailLexer

logger = get_logger(__name__)

_DEFAULT_OPTIONS: dict[str, Any] = {
    "kind": None,  # restrict lookup to one definition kind (function, type, ...)
    "heading_level": 2,
    "show_comment": True,
    "show_source": True,
    "link_code": True,
    "hover_previews": True,
    "hover_cards": True,
    "toc_label": None,
}

_PREVIEW_LINES = 8  # plain-text title tooltips
_PREVIEW_CHARS = 700
_CARD_LINES = 120  # scrollable hover cards
_CARD_CHARS = 20_000


def _token_class(ttype) -> str:
    while ttype is not None:
        cls = STANDARD_TYPES.get(ttype)
        if cls is not None:
            return cls
        ttype = ttype.parent
    return ""


def _body_preview(
    definition: Optional[Definition], max_lines: int = _PREVIEW_LINES, max_chars: int = _PREVIEW_CHARS
) -> Optional[str]:
    """A truncated source preview of a definition, for hover tooltips/cards."""
    if definition is None or not definition.clauses:
        return None
    text = definition.clauses[0].text.strip()
    if not text:
        return None
    lines = text.splitlines()
    preview = "\n".join(lines[:max_lines])
    if len(lines) > max_lines:
        preview += "\n…"
    return preview[:max_chars]


def _dedent_comment(comment: str) -> str:
    """Strip the comment's continuation indent so Markdown sees column 0.

    Doc comments indent continuation lines to align under ``/*!``; without
    dedenting, a paragraph after a blank line becomes a Markdown code block,
    and admonitions/lists cannot be expressed. The first line keeps only its
    own strip; relative indentation of later lines is preserved.
    """
    first, sep, rest = comment.partition("\n")
    return first.strip() + sep + textwrap.dedent(rest)


def _comment_summary(comment: Optional[str]) -> Optional[str]:
    """First sentence of a doc comment as tooltip-safe plain text."""
    if not comment:
        return None
    text = " ".join(comment.replace("`", "").replace("*", "").split())
    for stop in (". ", ".\n"):
        if stop in text:
            text = text[: text.index(stop) + 1]
            break
    return text[:140] or None


def _byte_to_str_spans(text: str, spans: list) -> list:
    """Convert byte-offset spans (docinfo / lsp-index convention) to str indices."""
    if text.isascii() or not spans:
        return spans
    data = text.encode("utf-8")

    def index(byte_offset: int) -> int:
        return len(data[:byte_offset].decode("utf-8", "ignore"))

    return [(index(span[0]), index(span[1]), *span[2:]) for span in spans]


def _lexical_tokens(text: str) -> list[tuple[int, int, str]]:
    """Fallback highlight segments from the bundled regex lexer."""
    tokens = []
    for pos, ttype, value in SailLexer().get_tokens_unprocessed(text):
        cls = _token_class(ttype) if ttype is not TextToken else ""
        tokens.append((pos, pos + len(value), cls))
    return tokens


def _highlight_linked(
    text: str, links: list[tuple[int, int, str]], tokens: list[tuple[int, int, str]] | None = None
) -> str:
    """Render source as HTML, wrapping link spans in <autoref> elements.

    ``links`` are non-overlapping (relative start, relative end, anchor)
    triples; ``tokens`` are non-overlapping (start, end, css class)
    highlight segments (semantic when a sail-lsp index is configured,
    lexical otherwise). Segments are split at every boundary so anchors and
    highlight spans nest cleanly.
    """
    if tokens is None:
        tokens = _lexical_tokens(text)
    # links: (start, end, anchor[, tooltip[, hover-card key]])
    links = sorted(
        (
            (start, end, anchor, *(tuple(rest) + (None, None))[:2])
            for start, end, anchor, *rest in links
        ),
        key=lambda link: link[:3],
    )
    tokens = sorted(tokens)

    bounds = {0, len(text)}
    for start, end, *_ in links:
        bounds.update((start, end))
    for start, end, _ in tokens:
        bounds.update((start, end))
    points = sorted(p for p in bounds if 0 <= p <= len(text))

    out = []
    open_until = None
    close_tag = ""
    link_i = 0
    token_i = 0
    for a, b in zip(points, points[1:]):
        if open_until is not None and a >= open_until:
            out.append(close_tag)
            open_until = None
        while link_i < len(links) and links[link_i][1] <= a:
            link_i += 1
        if open_until is None and link_i < len(links) and links[link_i][0] == a:
            _, end, anchor, tooltip, hover = links[link_i]
            out.append(f'<autoref identifier="{escape(anchor)}" optional>')
            if hover:
                # a hover card replaces the plain-text tooltip entirely
                out.append(f'<span data-sail-hover="{escape(hover)}">')
                close_tag = "</span></autoref>"
            elif tooltip:
                out.append(f'<span title="{escape(tooltip)}">')
                close_tag = "</span></autoref>"
            else:
                close_tag = "</autoref>"
            open_until = end
        while token_i < len(tokens) and tokens[token_i][1] <= a:
            token_i += 1
        cls = ""
        if token_i < len(tokens) and tokens[token_i][0] <= a < tokens[token_i][1]:
            cls = tokens[token_i][2]
        segment = escape(text[a:b])
        out.append(f'<span class="{cls}">{segment}</span>' if cls else str(segment))
    if open_until is not None:
        out.append(close_tag)
    return "".join(out)


class SailHandler(BaseHandler):
    """A mkdocstrings handler backed by Sail docinfo bundles."""

    name: ClassVar[str] = "sail"
    domain: ClassVar[str] = "sail"
    fallback_theme: ClassVar[str] = "material"

    extra_css = """
    /* Material 9.6.23+ lays md-code__content out as a grid, which turns
       every token span of a flat highlight stream into its own grid row;
       our blocks are flat token streams, so force normal flow */
    .md-typeset .sail-source pre > code { display: block !important; }
    .sail-source .autorefs { color: inherit; border-bottom: 1px dotted currentcolor; }
    .sail-source .autorefs:hover { border-bottom-style: solid; }
    .doc-sail-kind { font-size: 0.65em; font-weight: 400; opacity: 0.7; margin-right: 0.4em; }
    /* multi-line hover previews (signature + body) in Material tooltips */
    .md-tooltip__inner { white-space: pre-line; font-family: var(--md-code-font-family, monospace); font-size: 0.6rem; }
    /* IDE-style hover cards (positioned by sail-hover.js) */
    .sail-hovercard-float { position: fixed; z-index: 20; max-width: 42rem; max-height: 22rem; overflow: auto;
      background: var(--md-default-bg-color); color: var(--md-default-fg-color);
      border: 1px solid var(--md-default-fg-color--lightest); border-radius: 0.2rem;
      box-shadow: var(--md-shadow-z2, 0 0.2rem 0.5rem rgba(0,0,0,.2)); padding: 0.5rem 0.7rem; font-size: 0.7rem; }
    .sail-hovercard-float .sail-hovercard-doc { margin-bottom: 0.3rem; }
    /* EIP cards use the page's own typography (md-typeset) and a fixed
       box, applied at creation so every hover is the same size and is
       positioned exactly once */
    .sail-hovercard-float.sail-hovercard-eip { width: min(48rem, 92vw);
      height: min(30rem, 72vh); max-width: none; max-height: none; font-size: 0.8rem; }
    .sail-hovercard-float.sail-hovercard-eip table { display: block; overflow-x: auto; }
    .sail-hovercard-float.sail-hovercard-eip h2 { margin-top: 0; }
    .sail-hovercard-float pre { margin: 0; white-space: pre; overflow-x: auto; }
    /* live EIP cards highlight code client-side (highlight.js); map its
       token classes onto Material's code palette so they match the page */
    .sail-hovercard-eip .hljs-keyword, .sail-hovercard-eip .hljs-literal,
    .sail-hovercard-eip .hljs-built_in { color: var(--md-code-hl-keyword-color); }
    .sail-hovercard-eip .hljs-string, .sail-hovercard-eip .hljs-regexp { color: var(--md-code-hl-string-color); }
    .sail-hovercard-eip .hljs-number { color: var(--md-code-hl-number-color); }
    .sail-hovercard-eip .hljs-comment, .sail-hovercard-eip .hljs-doctag { color: var(--md-code-hl-comment-color); }
    .sail-hovercard-eip .hljs-title, .sail-hovercard-eip .hljs-title.function_,
    .sail-hovercard-eip .hljs-title.class_ { color: var(--md-code-hl-function-color); }
    .sail-hovercard-eip .hljs-type, .sail-hovercard-eip .hljs-symbol,
    .sail-hovercard-eip .hljs-meta { color: var(--md-code-hl-special-color); }
    .sail-hovercard-eip .hljs-attr, .sail-hovercard-eip .hljs-variable,
    .sail-hovercard-eip .hljs-params { color: var(--md-code-hl-variable-color); }
    .sail-hovercard-eip .hljs-operator, .sail-hovercard-eip .hljs-punctuation { color: var(--md-code-hl-operator-color); }
    """

    def __init__(self, config: Mapping[str, Any], base_dir: Path, **kwargs: Any) -> None:
        super().__init__(**kwargs)
        bundle = config.get("bundle")
        if bundle is None:
            raise CollectionError("The Sail handler needs a 'bundle' path (sail --doc output) in its configuration")
        self._bundle_path = Path(base_dir, bundle)
        self._bundle: Optional[Bundle] = None
        lsp_index = config.get("lsp_index")
        self._lsp_index_path = Path(base_dir, lsp_index) if lsp_index else None
        self._lsp_index: Optional[LspIndex] = None
        self._global_options: Mapping[str, Any] = config.get("options", {})

    @property
    def bundle(self) -> Bundle:
        if self._bundle is None:
            try:
                self._bundle = Bundle.load(self._bundle_path)
            except BundleError as error:
                raise CollectionError(str(error)) from error
        return self._bundle

    @property
    def lsp_index(self) -> Optional[LspIndex]:
        if self._lsp_index is None and self._lsp_index_path is not None:
            try:
                self._lsp_index = LspIndex.load(self._lsp_index_path)
            except (OSError, ValueError, json.JSONDecodeError) as error:
                raise CollectionError(f"Cannot load sail-lsp index {self._lsp_index_path}: {error}") from error
        return self._lsp_index

    # sail/sourceMap kinds -> docinfo bundle kinds for direct anchor lookup
    _REF_KINDS = {
        "type": "type",
        "union": "type",
        "struct": "type",
        "enum": "type",
        "register": "register",
        "mapping": "mapping",
        "let": "let",
        "constant": "let",
    }

    def _resolve_reference(self, ref: IndexReference) -> tuple[Optional[str], Optional[Definition]]:
        if ref.kind in ("function", "val"):
            try:
                definition = self.bundle.find(ref.name)
                return definition.anchor, definition
            except BundleError:
                return None, None
        mapped = self._REF_KINDS.get(ref.kind)
        if mapped is not None:
            try:
                definition = self.bundle.find(ref.name, kind=mapped)
                return definition.anchor, definition
            except BundleError:
                pass
        # constructors, enum members, and anything else docinfo has no
        # section for: anchor at whichever definition contains the target
        owner = self.bundle.definition_at(ref.target_file, ref.target_start)
        return (owner.anchor, owner) if owner else (None, None)

    def _tooltip(
        self, signature: Optional[str], definition: Optional[Definition], *, preview: bool
    ) -> Optional[str]:
        summary = _comment_summary(definition.comment) if definition is not None else None
        if signature and summary:
            head = f"{signature} — {summary}"
        else:
            head = signature or summary
        body = _body_preview(definition) if preview else None
        if head and body:
            return f"{head}\n\n{body}"
        return head or body

    def _clause_links(
        self,
        data: Definition,
        clause: Clause,
        *,
        preview: bool,
        cards: Optional[dict[str, tuple[Definition, Optional[str]]]] = None,
    ) -> list[tuple[int, int, str, Optional[str], Optional[str]]]:
        index = self.lsp_index

        def link_entry(start, end, anchor, target, signature):
            hover = None
            if cards is not None and target is not None:
                cards.setdefault(anchor, target)
                hover = anchor
            tooltip = None if hover else self._tooltip(signature, target, preview=preview)
            return (start, end, anchor, tooltip, hover)

        links: list[tuple[int, int, str, Optional[str], Optional[str]]] = []
        for start, end, record in clause.records_within(data.links):
            target = None
            try:
                target = self.bundle.find(record.identifier, kind=None if record.kind == "function" else record.kind)
            except BundleError:
                pass
            signature = index.signature(record.kind, record.identifier) if index is not None else None
            links.append(link_entry(start, end, record.anchor, target, signature))
        if index is not None and clause.file is not None and clause.start is not None and index.has_file(clause.file):
            for ref in index.references_within(clause.file, clause.start, clause.end):
                if any(ref.start < end and ref.end > start for start, end, *_ in links):
                    continue  # docinfo's compiler-proven links win
                anchor, target = self._resolve_reference(ref)
                if anchor is not None:
                    signature = index.signature(ref.kind, ref.name)
                    links.append(link_entry(ref.start, ref.end, anchor, target, signature))
        return sorted(links, key=lambda link: link[:3])

    def _hover_card(self, anchor: str, definition: Definition) -> str:
        """A hidden IDE-style hover card: doc comment + highlighted body."""
        parts = [f'<template class="sail-hovercard" data-anchor="{escape(anchor)}">']
        if definition.comment:
            parts.append(
                f'<div class="sail-hovercard-doc">'
                f'{self.do_convert_markdown(_dedent_comment(definition.comment).strip(), 6)}</div>'
            )
        preview = _body_preview(definition, max_lines=_CARD_LINES, max_chars=_CARD_CHARS)
        if preview:
            code = _highlight_linked(preview, [])
            parts.append(f'<div class="sail-hovercard-code highlight"><pre><code>{code}</code></pre></div>')
        parts.append("</template>")
        return "".join(parts)

    def _clause_tokens(self, clause: Clause) -> Optional[list[tuple[int, int, str]]]:
        index = self.lsp_index
        if index is not None and clause.file is not None and clause.start is not None and index.has_file(clause.file):
            tokens = index.tokens_within(clause.file, clause.start, clause.end)
            if tokens:
                return tokens
        return None  # fall back to the regex lexer

    def get_options(self, local_options: Mapping[str, Any]) -> Mapping[str, Any]:
        unknown = set(local_options) - set(_DEFAULT_OPTIONS)
        if unknown:
            logger.warning(f"Unknown Sail handler options: {', '.join(sorted(unknown))}")
        return {**_DEFAULT_OPTIONS, **self._global_options, **local_options}

    def collect(self, identifier: str, options: Mapping[str, Any]) -> Definition:
        try:
            return self.bundle.find(identifier, kind=options.get("kind"))
        except BundleError as error:
            raise CollectionError(str(error)) from error

    def get_aliases(self, identifier: str) -> tuple[str, ...]:
        # Called with the rendered heading id ("function-step"); alias the bare
        # identifier ("step") to it when the bare lookup resolves to the same
        # definition, so `[step][]` works in prose.
        kind, _, bare = identifier.partition("-")
        if not bare:
            return ()
        try:
            definition = self.bundle.find(bare)
        except (BundleError, CollectionError):
            return ()
        return (bare,) if definition.anchor == identifier else ()

    def render(self, data: Definition, options: Mapping[str, Any], *, locale: str | None = None) -> str:
        heading_level = int(options["heading_level"])
        parts = [f'<div class="doc doc-sail doc-{escape(data.kind)}">']
        heading = Markup('<span class="doc-sail-kind">{kind} </span><code>{id}</code>').format(
            kind=data.kind, id=data.identifier
        )
        attributes = {}
        signature = self.lsp_index.signature(data.kind, data.identifier) if self.lsp_index is not None else None
        if signature:
            attributes["title"] = signature
        parts.append(
            str(
                self.do_heading(
                    heading,
                    heading_level,
                    role=data.kind,
                    id=data.anchor,
                    toc_label=options.get("toc_label") or data.identifier,
                    **attributes,
                )
            )
        )
        if options["show_comment"] and data.comment:
            comment = _dedent_comment(data.comment).strip()
            comment = link_eip_references(comment, set(), hover=True)
            parts.append(str(self.do_convert_markdown(comment, heading_level + 1)))
        if options["show_source"]:
            cards: Optional[dict[str, Definition]] = {} if options["hover_cards"] else None
            for clause in data.clauses:
                links = (
                    self._clause_links(data, clause, preview=bool(options["hover_previews"]), cards=cards)
                    if options["link_code"]
                    else []
                )
                links = _byte_to_str_spans(clause.text, links)
                tokens = self._clause_tokens(clause)
                if tokens is not None:
                    tokens = _byte_to_str_spans(clause.text, tokens)
                code = _highlight_linked(clause.text, links, tokens=tokens)
                parts.append(f'<div class="sail-source language-sail highlight"><pre><code>{code}</code></pre></div>')
            if cards:
                parts.extend(self._hover_card(anchor, defn) for anchor, defn in sorted(cards.items()))
        parts.append("</div>")
        return "".join(parts)


def get_handler(
    theme: str,
    custom_templates: str | None = None,
    mdx: Any = (),
    mdx_config: Any = None,
    handler_config: Mapping[str, Any] | None = None,
    tool_config: Any = None,
    **kwargs: Any,
) -> SailHandler:
    """Return an instance of the Sail handler (mkdocstrings entry point)."""
    config_file = getattr(tool_config, "config_file_path", None) or (tool_config or {}).get("config_file_path")
    base_dir = Path(config_file).parent if config_file else Path.cwd()
    return SailHandler(
        config=handler_config or {},
        base_dir=base_dir,
        theme=theme,
        custom_templates=custom_templates,
        mdx=mdx,
        mdx_config=mdx_config or {},
    )
