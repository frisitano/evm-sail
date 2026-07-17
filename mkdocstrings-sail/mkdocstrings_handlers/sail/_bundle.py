"""Load and query Sail docinfo bundles.

A bundle is the ``doc.json`` produced by::

    sail --doc --doc-format identity --doc-embed plain --doc-embed-with-location

``--doc-embed plain`` embeds each definition's source text; adding
``--doc-embed-with-location`` also records its absolute character range in
the source file. Hyperlink entries (function calls and register uses) carry
absolute character ranges too, so use sites can be located *inside* the
embedded source by plain offset arithmetic.
"""

from __future__ import annotations

import base64 as _base64
import json
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterator, Optional

_ANCHOR_UNSAFE = re.compile(r"[^A-Za-z0-9_.-]")


def anchor_for(kind: str, identifier: str) -> str:
    """Stable anchor id, derived only from the definition kind and identifier."""
    return f"{kind}-{_ANCHOR_UNSAFE.sub('-', identifier)}"


@dataclass
class Link:
    """A use site of another definition within a source file."""

    kind: str  # "function", "register", or "mapping" (retargeted application)
    identifier: str
    start: int  # absolute character offset in the source file
    end: int

    @property
    def anchor(self) -> str:
        return anchor_for(self.kind, self.identifier)


@dataclass
class Clause:
    """One embedded source excerpt (a whole definition or a function clause)."""

    text: str
    comment: Optional[str] = None
    file: Optional[str] = None
    start: Optional[int] = None  # absolute character range when known
    end: Optional[int] = None

    def records_within(self, links: list[Link]) -> list[tuple[int, int, "Link"]]:
        """Return (relative start, relative end, link) for links inside this clause."""
        if self.start is None or self.end is None:
            return []
        out = []
        last_end = -1
        for link in sorted(links, key=lambda l: l.start):
            if link.start >= self.start and link.end <= self.end and link.start >= last_end:
                out.append((link.start - self.start, link.end - self.start, link))
                last_end = link.end
        return out

    def links_within(self, links: list[Link]) -> list[tuple[int, int, str]]:
        """Return (relative start, relative end, anchor) for links inside this clause."""
        return [(start, end, link.anchor) for start, end, link in self.records_within(links)]


@dataclass
class Definition:
    """A documented Sail definition resolved from a bundle."""

    kind: str
    identifier: str
    clauses: list[Clause]
    links: list[Link] = field(default_factory=list)

    @property
    def anchor(self) -> str:
        return anchor_for(self.kind, self.identifier)

    @property
    def comment(self) -> Optional[str]:
        for clause in self.clauses:
            if clause.comment:
                return clause.comment
        return None


class BundleError(Exception):
    """Raised when a bundle cannot be loaded or an identifier resolved."""


# (kind, JSON section key, JSON payload label) in lookup priority order
_SECTIONS = (
    ("function", "functions", "function"),
    ("mapping", "mappings", "mapping"),
    ("type", "types", "type"),
    ("register", "registers", "register"),
    ("let", "lets", "let"),
    ("val", "vals", "val"),
    ("anchor", "anchors", "anchor"),
    ("span", "spans", "span"),
)


class Bundle:
    """A parsed docinfo bundle."""

    def __init__(self, data: dict[str, Any], *, path: Optional[Path] = None):
        version = data.get("version")
        if version != 1:
            raise BundleError(f"Unsupported docinfo version {version!r} (expected 1) in {path or 'bundle'}")
        self._data = data
        self._embedding = data.get("embedding", "plain")
        self._path = path
        self._intervals: Optional[dict[str, list[tuple[int, int, Definition]]]] = None

    @classmethod
    def load(cls, path: str | Path) -> "Bundle":
        path = Path(path)
        try:
            with path.open() as handle:
                return cls(json.load(handle), path=path)
        except (OSError, json.JSONDecodeError) as error:
            raise BundleError(f"Cannot load docinfo bundle {path}: {error}") from error

    def _decode(self, contents: str) -> str:
        if self._embedding == "base64":
            return _base64.b64decode(contents).decode("utf-8")
        return contents

    def _clause_from_source(self, source: Any, comment: Optional[str]) -> Clause:
        if isinstance(source, str):  # Raw: pretty-printed, no location
            return Clause(text=self._decode(source), comment=comment)
        if isinstance(source, dict) and "contents" in source:
            loc = source.get("loc")
            if isinstance(loc, list) and len(loc) == 6:
                # loc = [line1, bol1, char1, line2, bol2, char2]
                return Clause(
                    text=self._decode(source["contents"]),
                    comment=comment,
                    file=source.get("file"),
                    start=loc[2],
                    end=loc[5],
                )
            return Clause(text=self._decode(source["contents"]), comment=comment)
        raise BundleError(
            "Definition source is a location reference without contents; "
            "regenerate the bundle with --doc-embed plain --doc-embed-with-location"
        )

    def _links_of(self, entry: dict[str, Any]) -> list[Link]:
        links = []
        for link in entry.get("links", []):
            loc = link.get("loc")
            if link.get("type") in ("function", "register") and isinstance(loc, list) and len(loc) == 2:
                kind, identifier = link["type"], link["id"]
                # Mapping applications look like function calls to the
                # hyperlink scanner; retarget them at the mapping's anchor.
                if (
                    kind == "function"
                    and identifier not in self._data.get("functions", {})
                    and identifier not in self._data.get("vals", {})
                    and identifier in self._data.get("mappings", {})
                ):
                    kind = "mapping"
                links.append(Link(kind=kind, identifier=identifier, start=loc[0], end=loc[1]))
        return links

    def _definition_from_entry(self, kind: str, label: str, identifier: str, entry: Any) -> Definition:
        # types and spans store the source payload directly; the rest nest it
        payload = entry.get(label) if isinstance(entry, dict) and label in entry else entry
        links = self._links_of(entry) if isinstance(entry, dict) else []

        if kind == "function":
            docs = payload if isinstance(payload, list) else [payload]
            clauses = [self._clause_from_source(doc["source"], doc.get("comment")) for doc in docs]
        elif kind == "mapping":
            clauses = [self._clause_from_source(doc["source"], doc.get("comment")) for doc in payload]
        elif kind in ("type", "span"):
            # a type's comment (docinfo >= Sail 0.20.3) sits beside the "type" key
            comment = entry.get("comment") if isinstance(entry, dict) else None
            clauses = [self._clause_from_source(payload, comment)]
        else:  # val, register, let, anchor
            clauses = [self._clause_from_source(payload["source"], payload.get("comment"))]
        return Definition(kind=kind, identifier=identifier, clauses=clauses, links=links)

    def find(self, identifier: str, kind: Optional[str] = None) -> Definition:
        """Resolve an identifier, optionally restricted to one definition kind."""
        for section_kind, section, label in _SECTIONS:
            if kind is not None and section_kind != kind:
                continue
            entry = self._data.get(section, {}).get(identifier)
            if entry is not None:
                return self._definition_from_entry(section_kind, label, identifier, entry)
        wanted = f"{kind} " if kind else ""
        raise BundleError(f"No {wanted}definition named {identifier!r} in docinfo bundle {self._path or ''}")

    def definition_at(self, file: str, offset: int) -> Optional[Definition]:
        """The definition whose source range contains (file, offset), if any.

        Used to re-anchor references docinfo has no section for (e.g. a
        union constructor resolves to the clause inside its owning type).
        """
        if self._intervals is None:
            self._intervals = {}
            for kind, identifier in self.identifiers():
                try:
                    definition = self.find(identifier, kind=kind)
                except BundleError:
                    continue
                for clause in definition.clauses:
                    if clause.file is not None and clause.start is not None and clause.end is not None:
                        self._intervals.setdefault(clause.file, []).append((clause.start, clause.end, definition))
        best = None
        for start, end, definition in self._intervals.get(file, []):
            if start <= offset < end and (best is None or end - start < best[0]):
                best = (end - start, definition)
        return best[1] if best else None

    def identifiers(self) -> Iterator[tuple[str, str]]:
        """Yield every (kind, identifier) pair in the bundle."""
        for section_kind, section, _ in _SECTIONS:
            for identifier in self._data.get(section, {}):
                yield section_kind, identifier
