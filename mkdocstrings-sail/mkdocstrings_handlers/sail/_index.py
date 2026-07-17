"""Consume a sail-lsp index (produced by ``sail-lsp-index``).

Provides semantic highlight tokens and identifier reference links clipped
to a definition's source range, in clause-relative offsets ready for the
renderer.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Optional

# LSP semantic token type -> Pygments short CSS class, so Material's
# existing pygments stylesheet colors the output.
_TOKEN_CLASSES = {
    "namespace": "nn",
    "type": "kt",
    "class": "nc",
    "enum": "kt",
    "interface": "kt",
    "struct": "kt",
    "typeParameter": "nv",
    "parameter": "n",
    "variable": "n",
    "property": "n",
    "enumMember": "no",
    "event": "n",
    "function": "nf",
    "method": "nf",
    "macro": "cp",
    "keyword": "k",
    "modifier": "k",
    "comment": "c",
    "string": "s",
    "number": "m",
    "regexp": "sr",
    "operator": "o",
}


@dataclass
class IndexReference:
    """A use site of a named definition, in clause-relative offsets."""

    start: int
    end: int
    name: str
    kind: str
    target_file: str
    target_start: int


class LspIndex:
    """A parsed sail-lsp index."""

    def __init__(self, data: dict[str, Any], *, path: Optional[Path] = None):
        if data.get("version") != 1:
            raise ValueError(f"Unsupported sail-lsp index version in {path or 'index'}")
        self._legend = data.get("legend", [])
        self._files = data.get("files", {})
        self._references = data.get("references", [])
        self._signatures = data.get("signatures", {})

    @classmethod
    def load(cls, path: str | Path) -> "LspIndex":
        path = Path(path)
        with path.open() as handle:
            return cls(json.load(handle), path=path)

    def has_file(self, file: Optional[str]) -> bool:
        return file is not None and file in self._files

    def signature(self, kind: str, name: str) -> Optional[str]:
        """The type signature recorded for a definition, if any.

        Signatures usually live on the val entry, so fall back through
        related kinds.
        """
        for k in (kind, "val", "function"):
            signature = self._signatures.get(f"{k}:{name}")
            if signature:
                return signature
        return None

    def tokens_within(self, file: str, start: int, end: int) -> list[tuple[int, int, str]]:
        """Semantic highlight segments for [start, end), clause-relative."""
        out = []
        for token_start, token_end, token_type in self._files.get(file, {}).get("tokens", []):
            if token_start >= end or token_end <= start:
                continue
            name = self._legend[token_type] if 0 <= token_type < len(self._legend) else ""
            cls = _TOKEN_CLASSES.get(name, "")
            out.append((max(token_start, start) - start, min(token_end, end) - start, cls))
        return sorted(out)

    def references_within(self, file: str, start: int, end: int) -> list[IndexReference]:
        """References whose use site lies fully inside [start, end), clause-relative."""
        out = []
        for ref in self._references:
            if ref["file"] != file or ref["start"] < start or ref["end"] > end:
                continue
            out.append(
                IndexReference(
                    start=ref["start"] - start,
                    end=ref["end"] - start,
                    name=ref["name"],
                    kind=ref["kind"],
                    target_file=ref["targetFile"],
                    target_start=ref["targetStart"],
                )
            )
        return sorted(out, key=lambda r: r.start)
