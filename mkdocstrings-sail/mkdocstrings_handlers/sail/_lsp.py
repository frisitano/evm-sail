"""Dump a sail-lsp index for the Sail mkdocstrings handler.

Drives a Sail language server over stdio and writes a single JSON index
containing per-file semantic tokens and name-resolved reference links from
the server's ``sail/sourceMap`` request. The handler consumes this index
(``lsp_index`` option) to upgrade code rendering from lexical to semantic
highlighting and to link identifier kinds the docinfo bundle cannot
(type references, constructor uses, …).

Usage (from the project root, so paths match the docinfo bundle)::

    sail-lsp-index --root . --output doc/lsp-index.json
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import threading
from pathlib import Path
from typing import Any, Optional

INDEX_VERSION = 1


def _utf16_to_str_index(line: str, units: int) -> int:
    """Convert a UTF-16 code-unit column (LSP default encoding) to a str index."""
    if units <= 0:
        return 0
    count = 0
    for i, ch in enumerate(line):
        if count >= units:
            return i
        count += 2 if ord(ch) > 0xFFFF else 1
    return len(line)


class _LineIndex:
    """Maps LSP (line, character) positions to absolute *byte* offsets.

    Sail's docinfo bundle counts offsets in bytes (OCaml lexing positions),
    so the index must too, or definitions preceded by any multi-byte
    character (e.g. an em dash in a comment) get misaligned spans.
    """

    def __init__(self, text: str):
        self.lines = text.split("\n")
        self.starts = [0]
        for line in self.lines[:-1]:
            self.starts.append(self.starts[-1] + len(line.encode("utf-8")) + 1)

    def offset(self, line: int, character: int) -> int:
        line = min(line, len(self.lines) - 1)
        line_text = self.lines[line]
        return self.starts[line] + len(line_text[: _utf16_to_str_index(line_text, character)].encode("utf-8"))


def decode_semantic_tokens(data: list[int], text: str) -> list[list[int]]:
    """Decode LSP relative-encoded semantic tokens to [start, end, type] offsets."""
    index = _LineIndex(text)
    tokens = []
    line = 0
    col = 0
    for i in range(0, len(data) - 4, 5):
        delta_line, delta_col, length, token_type, _modifiers = data[i : i + 5]
        if delta_line:
            line += delta_line
            col = delta_col
        else:
            col += delta_col
        start = index.offset(line, col)
        end = index.offset(line, col + length)
        if end > start:
            tokens.append([start, end, token_type])
    return tokens


class LspClient:
    """A minimal JSON-RPC-over-stdio LSP client (stdlib only)."""

    def __init__(self, command: list[str]):
        self.proc = subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        threading.Thread(target=self._drain_stderr, daemon=True).start()
        self._next_id = 0

    def _drain_stderr(self) -> None:
        assert self.proc.stderr is not None
        for _ in self.proc.stderr:
            pass

    def _send(self, message: dict[str, Any]) -> None:
        assert self.proc.stdin is not None
        data = json.dumps(message).encode()
        self.proc.stdin.write(b"Content-Length: %d\r\n\r\n%s" % (len(data), data))
        self.proc.stdin.flush()

    def _recv(self) -> dict[str, Any]:
        assert self.proc.stdout is not None
        length = None
        while True:
            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError("sail-lsp exited unexpectedly")
            if line == b"\r\n":
                break
            key, _, value = line.decode().partition(":")
            if key.strip().lower() == "content-length":
                length = int(value)
        assert length is not None
        return json.loads(self.proc.stdout.read(length))

    def notify(self, method: str, params: Any) -> None:
        self._send({"jsonrpc": "2.0", "method": method, "params": params})

    def request(self, method: str, params: Any) -> Any:
        self._next_id += 1
        rid = self._next_id
        self._send({"jsonrpc": "2.0", "id": rid, "method": method, "params": params})
        while True:
            message = self._recv()
            if message.get("id") == rid:
                if "error" in message:
                    raise RuntimeError(f"{method} failed: {message['error']}")
                return message.get("result")

    def close(self) -> None:
        try:
            self.request("shutdown", None)
            self.notify("exit", None)
        except Exception:
            pass
        self.proc.kill()


def _relative_to(root: Path, uri: str) -> Optional[str]:
    path = Path(uri.removeprefix("file://"))
    try:
        return str(path.relative_to(root))
    except ValueError:
        return None  # outside the workspace (e.g. Sail stdlib)


def project_files(sail_binary: str, root: Path, project: str, module: str, variables: list[str]) -> list[str]:
    """The project's resolved file closure, from ``sail --list-files``."""
    command = [sail_binary, "--project", project, "--list-files", module]
    for variable in variables:
        command += ["--variable", variable]
    result = subprocess.run(command, cwd=root, capture_output=True, text=True, check=True)
    return result.stdout.split()


def build_index(
    binary: str, root: Path, *, compiler_tokens: bool = True, files: list[str] | None = None
) -> dict[str, Any]:
    client = LspClient([binary, "--stdio"])
    try:
        init = client.request(
            "initialize",
            {
                "processId": None,
                "rootUri": root.as_uri(),
                "capabilities": {"general": {"positionEncodings": ["utf-16"]}},
                "initializationOptions": {"semanticCompilerTokens": compiler_tokens},
            },
        )
        legend = init["capabilities"]["semanticTokensProvider"]["legend"]["tokenTypes"]
        client.notify("initialized", {})

        # Open the files first: the server resolves its source graph (and
        # hence sail/sourceMap) from opened documents. Prefer the project
        # closure (--project); the workspace glob fallback can open files
        # outside the model, whose same-named definitions would pollute the
        # name-based reference graph.
        if files is None:
            files = sorted(str(p.relative_to(root)) for p in root.rglob("*.sail") if "_build" not in p.parts)
        texts: dict[str, str] = {}
        line_indexes: dict[str, _LineIndex] = {}
        for rel in files:
            path = root / rel
            if not path.exists():
                continue
            text = path.read_text()
            texts[rel] = text
            line_indexes[rel] = _LineIndex(text)
            client.notify(
                "textDocument/didOpen",
                {"textDocument": {"uri": path.as_uri(), "languageId": "sail", "version": 1, "text": text}},
            )

        entries = client.request("sail/sourceMap", {}) or []

        references = []
        signatures: dict[str, str] = {}
        for entry in entries:
            target_file = _relative_to(root, entry["location"]["uri"])
            if target_file is None:
                continue
            if entry.get("type"):
                signatures[f"{entry['kind']}:{entry['name']}"] = entry["type"]
            target_range = entry["location"]["range"]
            for ref in entry.get("references", []):
                ref_file = _relative_to(root, ref["uri"])
                if ref_file is None:
                    continue
                if ref_file == target_file and ref["range"] == target_range:
                    continue  # the defining occurrence itself
                references.append(
                    {
                        "name": entry["name"],
                        "kind": entry["kind"],
                        "file": ref_file,
                        "range": ref["range"],
                        "targetFile": target_file,
                        "targetRange": target_range,
                    }
                )

        file_tokens: dict[str, Any] = {}
        for rel, text in texts.items():
            result = client.request(
                "textDocument/semanticTokens/full", {"textDocument": {"uri": (root / rel).as_uri()}}
            )
            data = (result or {}).get("data", [])
            file_tokens[rel] = {"tokens": decode_semantic_tokens(data, text)}

        # resolve reference ranges to absolute character offsets
        resolved = []
        for ref in references:
            src = line_indexes.get(ref["file"])
            dst = line_indexes.get(ref["targetFile"])
            if src is None or dst is None:
                continue
            resolved.append(
                {
                    "name": ref["name"],
                    "kind": ref["kind"],
                    "file": ref["file"],
                    "start": src.offset(ref["range"]["start"]["line"], ref["range"]["start"]["character"]),
                    "end": src.offset(ref["range"]["end"]["line"], ref["range"]["end"]["character"]),
                    "targetFile": ref["targetFile"],
                    "targetStart": dst.offset(
                        ref["targetRange"]["start"]["line"], ref["targetRange"]["start"]["character"]
                    ),
                }
            )

        return {
            "version": INDEX_VERSION,
            "legend": legend,
            "files": file_tokens,
            "references": resolved,
            "signatures": signatures,
        }
    finally:
        client.close()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", default="sail_lsp", help="sail-lsp binary (default: sail_lsp on PATH)")
    parser.add_argument("--root", default=".", help="workspace root; paths are stored relative to it")
    parser.add_argument("--output", required=True, help="output JSON path")
    parser.add_argument("--no-compiler-tokens", action="store_true", help="skip compiler-derived token overlay")
    parser.add_argument("--project", help="a .sail_project file; index exactly its resolved file closure")
    parser.add_argument("--module", help="project module to resolve (with --project)")
    parser.add_argument("--variable", action="append", default=[], help="project variable NAME=VALUE (repeatable)")
    parser.add_argument("--sail", default="sail", help="sail binary for --list-files (default: sail on PATH)")
    parser.add_argument("files", nargs="*", help="root-relative .sail files to index (instead of the workspace glob)")
    args = parser.parse_args(argv)

    root = Path(args.root).resolve()
    files = args.files or None
    if args.project:
        if not args.module:
            parser.error("--project requires --module")
        files = project_files(args.sail, root, args.project, args.module, args.variable) + (args.files or [])

    index = build_index(
        args.binary,
        root,
        compiler_tokens=not args.no_compiler_tokens,
        files=files,
    )
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(index))
    tokens = sum(len(f["tokens"]) for f in index["files"].values())
    print(
        f"wrote {output}: {len(index['files'])} files, {tokens} tokens, {len(index['references'])} references",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
