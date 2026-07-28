"""Generate the derived pages in a Sail specification book.

Definition-adjacent prose lives in the sources, so no intermediary Markdown
files are needed:

- ``/*md ... */`` comments hold Markdown prose. They are ordinary Sail
  comments (ignored by the compiler, preserved by ``sail --fmt``) and are
  interleaved with definitions by source position — a block at the top of a
  file with a ``# Title`` heading is the module doc (and names the page), a
  ``## Section`` block between definitions opens a section, and so on.
- ``/*! ... */`` doc comments attach to the following definition and are
  rendered under its heading, as usual.
- Authored section overviews live directly at their canonical URLs as
  ``BOOK/docs/reference/<source-directory>/index.md``.

For each source file containing definitions, a page is emitted with the
``/*md`` blocks and ``::: name`` directives in source order. Identifiers
already rendered by hand-authored pages under ``docs/`` are skipped, so
authored chapters and generated pages can coexist.

Generated files are recorded in ``BOOK/.build/generated-manifest.json``.
Before each run, only files listed in that manifest are removed; authored
pages sharing the ``docs/reference`` tree are never deleted.

Usage::

    sail-book-gen --root . --project model.sail_project --module core \
                  --book book
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from pathlib import Path
from typing import Optional

from ._bundle import Bundle
from ._eips import link_eip_references
from ._lsp import project_files

import posixpath

import markdown as _markdown
from markupsafe import escape as _escape
from pygments import highlight as _pyg_highlight
from pygments.formatters import HtmlFormatter as _HtmlFormatter
from pygments.lexers import get_lexer_by_name as _get_lexer

_ASSETS_DIR = Path(__file__).parent / "assets"

_HEADING = re.compile(r"^#\s+(.+)$", re.M)


def markdown_blocks(text: str) -> list[tuple[int, str]]:
    """Extract (offset, markdown) for each ``/*md ... */`` block.

    Handles Sail's nested block comments; the marker must be followed by
    whitespace.
    """
    blocks = []
    i = 0
    while True:
        i = text.find("/*md", i)
        if i < 0:
            return blocks
        after = i + 4
        if after < len(text) and text[after] not in " \t\r\n":
            i = after
            continue
        depth = 1
        j = after
        while j < len(text) - 1 and depth:
            if text[j : j + 2] == "/*":
                depth += 1
                j += 2
            elif text[j : j + 2] == "*/":
                depth -= 1
                j += 2
            else:
                j += 1
        content = text[after : j - 2] if depth == 0 else text[after:]
        blocks.append((i, content.strip("\r\n")))
        i = j


def page_items(bundle: Bundle, file: str, text: str, excluded: set[str]) -> list[tuple[str, ...]]:
    """('md', text) and ('def', kind, name) entries for a file, in source order."""
    entries: list[tuple[int, tuple[str, ...]]] = [(offset, ("md", block)) for offset, block in markdown_blocks(text)]
    function_names = {name for kind, name in bundle.identifiers() if kind == "function"}
    for kind, name in bundle.identifiers():
        if kind in ("anchor", "span") or (kind == "val" and name in function_names) or name in excluded:
            continue
        definition = bundle.find(name, kind=kind)
        for clause in definition.clauses:
            if clause.file == file and clause.start is not None:
                entries.append((clause.start, ("def", kind, name)))
                break
    return [item for _, item in sorted(entries, key=lambda entry: entry[0])]


def render_page(file: str, items: list[tuple[str, ...]]) -> tuple[str, str]:
    """Render a page; returns (nav title, markdown). The first ``/*md`` block's
    leading ``# heading`` names the page; otherwise the file path does."""
    title = Path(file).stem
    out = []
    if not (items and items[0][0] == "md" and _HEADING.match(items[0][1])):
        out.append(f"# `{file}`\n")
    else:
        title = _HEADING.match(items[0][1]).group(1).strip()
    for item in items:
        if item[0] == "md":
            out.append(link_eip_references(item[1], set(), hover=True) + "\n")
        else:
            _, kind, name = item
            out.append(f"::: {name}\n    options:\n      kind: {kind}\n")
    return title, "\n".join(out)


def lean_page_items(text: str) -> list[tuple[str, str]]:
    """Split a generated Lean file into ('md', prose) and ('code', chunk) items.

    ``/-! ... -/`` module docstrings and ``/-- ... -/`` definition
    docstrings (both carrying the specification prose in extracted code)
    become Markdown; everything between is Lean source."""
    items: list[tuple[str, str]] = []
    code: list[str] = []
    lines = text.split("\n")
    i = 0

    def flush_code() -> None:
        chunk = "\n".join(code).strip("\n")
        code.clear()
        if chunk:
            items.append(("code", chunk))

    while i < len(lines):
        line = lines[i]
        stripped = line.lstrip()
        marker = next((m for m in ("/-!", "/--") if stripped.startswith(m)), None)
        if marker and not stripped.startswith("/---"):
            flush_code()
            block: list[str] = [stripped[len(marker):]]
            while not block[-1].rstrip().endswith("-/"):
                i += 1
                if i >= len(lines):
                    break
                block.append(lines[i])
            block[-1] = block[-1].rstrip()[: -len("-/")]
            prose = "\n".join(part.strip() if n == 0 else part for n, part in enumerate(block)).strip()
            items.append(("md", prose))
        else:
            code.append(line)
        i += 1
    flush_code()
    return items


# --- Lean extraction: identifier links and hover cards -------------------
#
# The extraction preserves Sail identifier names verbatim, so a build-time
# index over the extracted modules gives every definition's page,
# docstring, and body. Identifier uses in Lean source link to their
# definition site and carry the same IDE-style hover cards as Sail code.

_LEAN_DEF = re.compile(
    r"^(?:noncomputable\s+)?(?:private\s+)?(?:protected\s+)?(?:partial\s+)?(?:unsafe\s+)?"
    r"(def|abbrev|inductive|structure|theorem|instance|opaque)\s+([A-Za-z_][A-Za-z0-9_'!?]*)",
    re.M,
)
_LEAN_CTOR = re.compile(r"^\s*\|\s*([A-Za-z_][A-Za-z0-9_']*)", re.M)
_LEAN_CARD_LINES = 120

def lean_page_url(relpath) -> str:
    return "extraction/lean/" + relpath.with_suffix("").as_posix() + "/"


def lean_definition_index(lean_files: list[Path], lean_root: Path) -> dict[str, dict]:
    """name -> {url, anchor, kind, doc, body} over all extracted modules."""
    index: dict[str, dict] = {}
    for f in lean_files:
        url = lean_page_url(f.relative_to(lean_root))
        items = lean_page_items(f.read_text())
        for i, (kind, chunk) in enumerate(items):
            if kind != "md" and (matches := list(_LEAN_DEF.finditer(chunk))):
                for n, m in enumerate(matches):
                    name = m.group(2)
                    if name in index:
                        continue
                    body = chunk[m.start() : matches[n + 1].start() if n + 1 < len(matches) else len(chunk)]
                    body = body.rstrip()
                    doc = ""
                    if n == 0 and i > 0 and items[i - 1][0] == "md" and not items[i - 1][1].startswith("# "):
                        doc = items[i - 1][1]
                    entry = {"url": url, "anchor": f"lean-{name}", "kind": m.group(1), "doc": doc, "body": body}
                    index[name] = entry
                    if m.group(1) in ("inductive", "structure"):
                        for cm in _LEAN_CTOR.finditer(body):
                            index.setdefault(cm.group(1), {**entry, "doc": "", "kind": "constructor"})
    return index


def _lean_href(entry: dict, page_url: str) -> str:
    if entry["url"] == page_url:
        return "#" + entry["anchor"]
    rel = posixpath.relpath(entry["url"].rstrip("/"), page_url.rstrip("/"))
    return rel + "/#" + entry["anchor"]


def lean_linked_html(chunk: str, index: dict[str, dict], page_url: str) -> str:
    """A Lean code chunk as highlighted HTML with definition links/anchors."""
    from pygments.token import Name as _Name
    from ._handler import _token_class

    def_heads = {m.start(2): m.group(2) for m in _LEAN_DEF.finditer(chunk)}
    out = []
    for pos, ttype, value in _get_lexer("lean4").get_tokens_unprocessed(chunk):
        cls = _token_class(ttype)
        piece = str(_escape(value))
        if cls:
            piece = f'<span class="{cls}">{piece}</span>'
        if def_heads.get(pos) == value:
            piece = f'<span id="lean-{value}">{piece}</span>'
        elif value in index and ttype in _Name:
            entry = index[value]
            piece = (
                f'<a class="autorefs" href="{_lean_href(entry, page_url)}"'
                f' data-sail-hover="{entry["anchor"]}">{piece}</a>'
            )
        out.append(piece)
    return (
        '<div class="highlight sail-source sail-lean-source"><pre><code>'
        + "".join(out)
        + "</code></pre></div>"
    )


def lean_card_html(entry: dict) -> str:
    """The hover-card fragment for a Lean definition: docstring + body."""
    parts = []
    if entry["doc"]:
        doc = _markdown.markdown(entry["doc"], extensions=["extra"])
        parts.append(f'<div class="sail-hovercard-doc">{doc}</div>')
    body = "\n".join(entry["body"].splitlines()[:_LEAN_CARD_LINES])
    code = _pyg_highlight(body, _get_lexer("lean4"), _HtmlFormatter(nowrap=True)).rstrip()
    parts.append(f'<div class="sail-hovercard-code highlight"><pre><code>{code}</code></pre></div>')
    return "".join(parts)


def render_lean_page(
    name: str,
    text: str,
    index: Optional[dict[str, dict]] = None,
    page_url: Optional[str] = None,
) -> tuple[str, str]:
    """Render an extracted Lean module; returns (title, markdown)."""
    items = lean_page_items(text)
    title = name
    out = []
    first = next((i for i, item in enumerate(items) if item[0] == "md"), None)
    if first is not None and _HEADING.match(items[first][1]):
        title = _HEADING.match(items[first][1]).group(1).strip()
        # the title heading must open the page (MkDocs reads the page
        # title from the leading block), ahead of the import prelude
        items.insert(0, items.pop(first))
    else:
        out.append(f"# `{name}.lean`\n")
    for kind, chunk in items:
        if kind == "md":
            out.append(link_eip_references(chunk, set(), hover=True) + "\n")
        elif index is not None and page_url is not None:
            out.append(lean_linked_html(chunk, index, page_url) + "\n")
        else:
            out.append("```lean4\n" + chunk + "\n```\n")
    return title, "\n".join(out)


_MANIFEST = Path(".build/generated-manifest.json")


def _manifest_entries(book: Path) -> list[str]:
    manifest = book / _MANIFEST
    if not manifest.exists():
        return []
    data = json.loads(manifest.read_text())
    if not isinstance(data, dict) or not isinstance(data.get("files"), list):
        raise ValueError(f"invalid generated-file manifest: {manifest}")
    if not all(isinstance(path, str) for path in data["files"]):
        raise ValueError(f"invalid generated-file manifest paths: {manifest}")
    return data["files"]


def _generated_path(book: Path, relative: str) -> Path:
    path = book / relative
    try:
        path.resolve().relative_to(book.resolve())
    except ValueError as error:
        raise ValueError(f"generated path escapes the book: {relative}") from error
    return path


def clean_generated(book: Path) -> int:
    """Remove only files owned by the previous generator run."""
    removed = 0
    for relative in _manifest_entries(book):
        path = _generated_path(book, relative)
        if path.is_file() or path.is_symlink():
            path.unlink()
            removed += 1
            parent = path.parent
            while parent != book and parent != book / "docs":
                try:
                    parent.rmdir()
                except OSError:
                    break
                parent = parent.parent
    manifest = book / _MANIFEST
    if manifest.exists():
        manifest.unlink()
    return removed


class GeneratedFiles:
    """Manifest-backed writer which refuses to overwrite authored files."""

    def __init__(self, book: Path):
        self.book = book
        clean_generated(book)
        self.files: set[str] = set()

    def _relative(self, path: Path) -> str:
        try:
            return path.resolve().relative_to(self.book.resolve()).as_posix()
        except ValueError as error:
            raise ValueError(f"generated path escapes the book: {path}") from error

    def _claim(self, path: Path) -> str:
        relative = self._relative(path)
        if path.exists() and relative not in self.files:
            raise FileExistsError(
                f"refusing to replace authored book file with generated output: {path}"
            )
        path.parent.mkdir(parents=True, exist_ok=True)
        self.files.add(relative)
        self._save()
        return relative

    def _save(self) -> None:
        manifest = self.book / _MANIFEST
        manifest.parent.mkdir(parents=True, exist_ok=True)
        manifest.write_text(
            json.dumps({"version": 1, "files": sorted(self.files)}, indent=2) + "\n"
        )

    def write_text(self, path: Path, text: str) -> None:
        self._claim(path)
        path.write_text(text)

    def copy(self, source: Path, destination: Path) -> None:
        self._claim(destination)
        shutil.copy(source, destination)


_CONFIG_TEMPLATE = """site_name: "{site_name}"
use_directory_urls: false
theme:
  name: material
  features:
    - content.code.copy
    - content.tooltips
    - navigation.indexes
    - navigation.top
    - search.highlight
plugins:
  - search
  - sail-book
  - literate-nav:
      nav_file: SUMMARY.md
  - autorefs:
      link_titles: false
  - mkdocstrings:
      default_handler: sail
      handlers:
        sail:
          bundle: .build/doc.json
          lsp_index: .build/lsp-index.json
markdown_extensions:
  - admonition
  - attr_list
  - toc:
      permalink: true
  - pymdownx.superfences
extra:
  version:
    provider: mike
extra_css:
  - assets/generated/performance-dashboard.css
extra_javascript:
  - assets/generated/marked.min.js
  - assets/generated/highlight.min.js
  - assets/generated/sail-hover.js
  - assets/generated/performance-dashboard.js
"""


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", default=".", help="project root (paths must match the docinfo bundle)")
    parser.add_argument("--project", help="a .sail_project file")
    parser.add_argument("--module", help="project module to resolve")
    parser.add_argument("--variable", action="append", default=[], help="project variable NAME=VALUE (repeatable)")
    parser.add_argument("--sail", default="sail", help="sail binary for --list-files")
    parser.add_argument(
        "--book",
        required=True,
        help="book directory (holds mkdocs.yml, docs/, and .build/doc.json)",
    )
    parser.add_argument("--site-name", default="Sail Specification")
    parser.add_argument("--lean", help="extracted Lean project directory; renders an extraction section")
    parser.add_argument("--no-config", action="store_true", help="do not (re)write mkdocs.yml")
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove manifest-owned generated files and exit",
    )
    args = parser.parse_args(argv)

    book = Path(args.book)
    if args.clean:
        print(f"removed {clean_generated(book)} generated book files", file=sys.stderr)
        return 0
    if not args.project or not args.module:
        parser.error("--project and --module are required unless --clean is used")

    root = Path(args.root).resolve()
    bundle = Bundle.load(book / ".build/doc.json")
    files = project_files(args.sail, root, args.project, args.module, args.variable)
    generated = GeneratedFiles(book)

    # identifiers rendered by hand-authored pages are not re-rendered
    excluded: set[str] = set()
    for page in (book / "docs").rglob("*.md"):
        for match in re.finditer(r"^::: (\S+)", page.read_text(), re.M):
            excluded.add(match.group(1))

    pages = []
    definitions = 0
    for file in files:
        source = root / file
        if not source.exists():
            continue
        items = page_items(bundle, file, source.read_text(), excluded)
        if not items:
            continue
        definitions += sum(1 for item in items if item[0] == "def")
        title, markdown = render_page(file, items)
        page = str(Path(file).with_suffix(".md"))
        path = book / "docs/reference" / page
        generated.write_text(path, markdown)
        pages.append((file, "reference/" + page, title))

    # extracted Lean modules become literate pages under extraction/lean/
    lean_files: list[Path] = []
    if args.lean:
        lean_root = Path(args.lean)
        lean_files = sorted(
            f for f in lean_root.rglob("*.lean") if ".lake" not in f.parts and f.name != "lakefile.lean"
        )
        lean_index = lean_definition_index(lean_files, lean_root)
        for f in lean_files:
            relpath = f.relative_to(lean_root)
            title, page_md = render_lean_page(f.stem, f.read_text(), lean_index, lean_page_url(relpath))
            path = book / "docs/extraction/lean" / relpath.with_suffix(".md")
            generated.write_text(path, page_md)
        cards_dir = book / "docs/assets/generated/lean-cards"
        written: set[str] = set()
        for entry in lean_index.values():
            if entry["anchor"] not in written:
                written.add(entry["anchor"])
                generated.write_text(
                    cards_dir / f"{entry['anchor']}.html", lean_card_html(entry)
                )
        if lean_files:
            print(
                f"rendered {len(lean_files)} Lean extraction pages, {len(written)} hover cards",
                file=sys.stderr,
            )

    assets = book / "docs/assets/generated"
    for pattern in ("*.js", "*.css"):
        for asset in _ASSETS_DIR.glob(pattern):
            generated.copy(asset, assets / asset.name)

    # a home page is required for the site root; generate one if not authored
    index = book / "docs/index.md"
    if not index.exists():
        toc = "\n".join(f"- [{title}]({page})" for _, page, title in pages)
        generated.write_text(index, f"# {args.site_name}\n\n## Modules\n\n" + toc + "\n")

    # navigation lives in docs/SUMMARY.md (mkdocs-literate-nav); generate a
    # default covering everything when none is authored
    summary = book / "docs/SUMMARY.md"
    if not summary.exists():
        generated.write_text(summary, "- [Home](index.md)\n- *.md\n- reference/*\n")

    config = book / "mkdocs.yml"
    if not args.no_config and not config.exists():
        generated.write_text(config, _CONFIG_TEMPLATE.format(site_name=args.site_name))

    print(
        f"generated {len(pages)} pages covering {definitions} definitions"
        + (f" (excluded {len(excluded)} hand-rendered ids)" if excluded else ""),
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
