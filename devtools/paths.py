"""Shared repository paths for development tooling."""

from __future__ import annotations

import os
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def temporary_root() -> Path:
    """Return the workspace-local directory for disposable tool artifacts."""
    root = Path(os.environ.get("AGENT_TMPDIR", REPO_ROOT / ".agent-tmp"))
    root.mkdir(parents=True, exist_ok=True)
    return root
