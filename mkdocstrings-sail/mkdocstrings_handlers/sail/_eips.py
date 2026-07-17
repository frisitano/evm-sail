"""EIP reference linking.

``EIP-N`` mentions in specification prose become links to
``https://eips.ethereum.org/EIPS/eip-N``, optionally tagged so the hover
script fetches the EIP's Markdown source at view time and renders it
client-side (there is no build-time EIP content).
"""

from __future__ import annotations

import re

_EIP_REF = re.compile(r"(?<![\w\[`-])EIP-(\d+)\b(?!\]|[\w-])")
_CODE_SPLIT = re.compile(r"(```.*?```|`[^`]*`)", re.S)

EIP_URL = "https://eips.ethereum.org/EIPS/eip-{n}"


def link_eip_references(markdown: str, seen: set[int], *, hover: bool) -> str:
    """Wrap bare ``EIP-N`` mentions in links (skipping code spans/fences).

    With ``hover`` set, links carry a ``data-sail-hover`` attribute keyed
    ``eip-N`` (via ``attr_list``) so the hover script reveals the EIP card.
    Numbers substituted are added to ``seen``.
    """

    def replace(match: re.Match) -> str:
        n = int(match.group(1))
        seen.add(n)
        attrs = f'{{: data-sail-hover="eip-{n}" .sail-eip }}' if hover else ""
        return f"[EIP-{n}]({EIP_URL.format(n=n)}){attrs}"

    parts = _CODE_SPLIT.split(markdown)
    return "".join(part if i % 2 else _EIP_REF.sub(replace, part) for i, part in enumerate(parts))
