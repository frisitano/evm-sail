#!/usr/bin/env python3
"""Provision and verify the repository-pinned EEST fixture corpus."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import tarfile
import tempfile
import tomllib
from collections.abc import Iterator, Sequence
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from urllib.request import urlopen

from devtools.build_support import BuildSupportError

TREE_DIGEST_SCHEMA = 1
BUFFER_SIZE = 1024 * 1024
DOWNLOAD_TIMEOUT_SECONDS = 60


@dataclass(frozen=True)
class CorpusManifest:
    schema_version: int
    extractor_schema: int
    name: str
    version: str
    source_tag: str
    source_commit: str
    t8n_version: str
    generation_args: str
    archive_url: str
    archive_sha256: str
    archive_root: str
    include_directories: tuple[str, ...]
    tree_sha256: str
    expected_json_files: int
    expected_cases: int


@dataclass(frozen=True)
class CorpusInventory:
    tree_sha256: str
    json_files: int
    embedded_cases: int


def load_manifest(path: Path) -> CorpusManifest:
    data = tomllib.loads(path.read_text(encoding="utf-8"))
    try:
        corpus = data["corpus"]
        source = data["source"]
        archive = data["archive"]
        inventory = data["inventory"]
        manifest = CorpusManifest(
            schema_version=int(data["schema_version"]),
            extractor_schema=int(data["extractor_schema"]),
            name=str(corpus["name"]),
            version=str(corpus["version"]),
            source_tag=str(source["tag"]),
            source_commit=str(source["commit"]),
            t8n_version=str(source["t8n_version"]),
            generation_args=str(source["generation_args"]),
            archive_url=str(archive["url"]),
            archive_sha256=_sha256_value(str(archive["sha256"])),
            archive_root=str(archive["root"]),
            include_directories=tuple(str(item) for item in archive["include_directories"]),
            tree_sha256=_sha256_value(str(inventory["tree_sha256"])),
            expected_json_files=int(inventory["json_files"]),
            expected_cases=int(inventory["embedded_cases"]),
        )
    except (KeyError, TypeError, ValueError) as error:
        raise BuildSupportError(f"{path}: invalid corpus manifest: {error}") from error
    if manifest.schema_version != 1:
        raise BuildSupportError(f"{path}: unsupported manifest schema {manifest.schema_version}")
    if manifest.extractor_schema != TREE_DIGEST_SCHEMA:
        raise BuildSupportError(f"{path}: unsupported extractor schema {manifest.extractor_schema}")
    if not manifest.include_directories:
        raise BuildSupportError(f"{path}: archive.include_directories is empty")
    for entry in (manifest.archive_root, *manifest.include_directories):
        relative = PurePosixPath(entry)
        if relative.is_absolute() or ".." in relative.parts:
            raise BuildSupportError(f"{path}: unsafe archive path: {entry!r}")
    if manifest.expected_json_files <= 0 or manifest.expected_cases <= 0:
        raise BuildSupportError(f"{path}: expected inventory must be positive")
    return manifest


def _sha256_value(value: str) -> str:
    normalized = value.removeprefix("sha256:").lower()
    if len(normalized) != 64 or any(c not in "0123456789abcdef" for c in normalized):
        raise ValueError(f"invalid SHA-256: {value!r}")
    return normalized


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(BUFFER_SIZE), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _count_embedded_cases(value: object) -> int:
    if isinstance(value, dict):
        count = int("statelessInputBytes" in value and "statelessOutputBytes" in value)
        return count + sum(_count_embedded_cases(item) for item in value.values())
    if isinstance(value, list):
        return sum(_count_embedded_cases(item) for item in value)
    return 0


def fixture_paths(root: Path, include_directories: Sequence[str]) -> Iterator[Path]:
    for directory in include_directories:
        base = root / directory
        if not base.is_dir():
            raise BuildSupportError(f"corpus directory is missing: {base}")
        yield from sorted(base.rglob("*.json"))


def inventory_corpus(root: Path, include_directories: Sequence[str]) -> CorpusInventory:
    """Hash fixture paths/content and count embedded input/output pairs."""

    digest = hashlib.sha256()
    json_files = 0
    embedded_cases = 0
    for path in fixture_paths(root, include_directories):
        relative = path.relative_to(root).as_posix().encode("utf-8")
        digest.update(len(relative).to_bytes(8, "big"))
        digest.update(relative)
        file_digest = hashlib.sha256()
        with path.open("rb") as stream:
            data = stream.read()
        file_digest.update(data)
        digest.update(file_digest.digest())
        json_files += 1
        try:
            embedded_cases += _count_embedded_cases(json.loads(data))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise BuildSupportError(f"invalid fixture JSON {path}: {error}") from error
    return CorpusInventory(digest.hexdigest(), json_files, embedded_cases)


def inventory_archive(archive_path: Path, manifest: CorpusManifest) -> CorpusInventory:
    """Inventory included fixtures directly from an authenticated archive."""

    digest = hashlib.sha256()
    json_files = 0
    embedded_cases = 0
    root = PurePosixPath(manifest.archive_root)
    prefixes = tuple(root / directory for directory in manifest.include_directories)
    with tarfile.open(archive_path, mode="r:gz") as archive:
        members = _safe_members(archive)
        included = []
        for member in members:
            path = PurePosixPath(member.name)
            if not member.isfile() or path.suffix != ".json":
                continue
            if any(path.is_relative_to(prefix) for prefix in prefixes):
                included.append((path.relative_to(root), member))
        for relative, member in sorted(included, key=lambda item: item[0].as_posix()):
            stream = archive.extractfile(member)
            if stream is None:
                raise BuildSupportError(f"cannot read archive member: {member.name}")
            data = stream.read()
            encoded_path = relative.as_posix().encode("utf-8")
            digest.update(len(encoded_path).to_bytes(8, "big"))
            digest.update(encoded_path)
            digest.update(hashlib.sha256(data).digest())
            json_files += 1
            try:
                embedded_cases += _count_embedded_cases(json.loads(data))
            except (UnicodeDecodeError, json.JSONDecodeError) as error:
                raise BuildSupportError(
                    f"invalid fixture JSON in archive {member.name}: {error}"
                ) from error
    return CorpusInventory(digest.hexdigest(), json_files, embedded_cases)


def verify_corpus(root: Path, manifest: CorpusManifest) -> CorpusInventory:
    inventory = inventory_corpus(root, manifest.include_directories)
    mismatches: list[str] = []
    if inventory.tree_sha256 != manifest.tree_sha256:
        mismatches.append(f"tree SHA-256 {inventory.tree_sha256} != {manifest.tree_sha256}")
    if inventory.json_files != manifest.expected_json_files:
        mismatches.append(f"JSON files {inventory.json_files} != {manifest.expected_json_files}")
    if inventory.embedded_cases != manifest.expected_cases:
        mismatches.append(f"embedded cases {inventory.embedded_cases} != {manifest.expected_cases}")
    if mismatches:
        raise BuildSupportError("corpus verification failed:\n  " + "\n  ".join(mismatches))
    return inventory


def _safe_members(archive: tarfile.TarFile) -> list[tarfile.TarInfo]:
    members = archive.getmembers()
    for member in members:
        path = PurePosixPath(member.name)
        if path.is_absolute() or ".." in path.parts:
            raise BuildSupportError(f"archive contains unsafe path: {member.name!r}")
        if member.issym() or member.islnk() or member.isdev():
            raise BuildSupportError(f"archive contains unsupported special entry: {member.name!r}")
    return members


def extract_archive(archive_path: Path, destination: Path) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive_path, mode="r:gz") as archive:
        archive.extractall(destination, members=_safe_members(archive), filter="data")


def _download(url: str, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    with (
        urlopen(url, timeout=DOWNLOAD_TIMEOUT_SECONDS) as response,
        destination.open("wb") as output,
    ):
        shutil.copyfileobj(response, output, length=BUFFER_SIZE)


def provision_corpus(
    manifest: CorpusManifest, destination: Path, cache_directory: Path
) -> CorpusInventory:
    """Verify a warm tree, otherwise download and atomically provision it."""

    if destination.exists():
        if not destination.is_dir():
            raise BuildSupportError(
                f"corpus destination exists and is not a directory: {destination}"
            )
        try:
            return verify_corpus(destination, manifest)
        except BuildSupportError as error:
            raise BuildSupportError(
                f"refusing to replace invalid corpus directory {destination}; "
                "remove it explicitly or select a fresh EEST_CORPUS path. "
                f"Verification failed: {error}"
            ) from error

    cache_directory.mkdir(parents=True, exist_ok=True)
    archive_path = cache_directory / f"{manifest.name}-{manifest.version}.tar.gz"
    if not archive_path.is_file() or file_sha256(archive_path) != manifest.archive_sha256:
        temporary_archive = archive_path.with_suffix(".download")
        temporary_archive.unlink(missing_ok=True)
        _download(manifest.archive_url, temporary_archive)
        actual = file_sha256(temporary_archive)
        if actual != manifest.archive_sha256:
            temporary_archive.unlink(missing_ok=True)
            raise BuildSupportError(f"archive SHA-256 {actual} != {manifest.archive_sha256}")
        temporary_archive.replace(archive_path)

    staging_parent = destination.parent
    staging_parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix=f".{destination.name}.extract-", dir=staging_parent
    ) as temporary:
        extracted = Path(temporary) / "archive"
        extract_archive(archive_path, extracted)
        source_root = extracted / manifest.archive_root
        staged = Path(temporary) / "corpus"
        staged.mkdir()
        for directory in manifest.include_directories:
            source = source_root / directory
            if not source.is_dir():
                raise BuildSupportError(f"archive corpus directory is missing: {source}")
            shutil.copytree(source, staged / directory)
        inventory = verify_corpus(staged, manifest)
        staged.rename(destination)
        return inventory


def manifest_cache_key(path: Path, manifest: CorpusManifest) -> str:
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    return f"corpus-{manifest.name}-{manifest.version}-schema{TREE_DIGEST_SCHEMA}-{digest}"


def _print_inventory(inventory: CorpusInventory) -> None:
    print(f"tree_sha256={inventory.tree_sha256}")
    print(f"json_files={inventory.json_files}")
    print(f"embedded_cases={inventory.embedded_cases}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=Path("config/corpora/tests-zkevm.toml"))
    subparsers = parser.add_subparsers(dest="command", required=True)

    verify = subparsers.add_parser("verify")
    verify.add_argument("root", type=Path)

    inventory = subparsers.add_parser("inventory")
    inventory.add_argument("root", type=Path)

    archive_inventory = subparsers.add_parser("archive-inventory")
    archive_inventory.add_argument("archive", type=Path)

    provision = subparsers.add_parser("provision")
    provision.add_argument("root", type=Path)
    provision.add_argument("--cache", type=Path, required=True)

    cache_key = subparsers.add_parser("cache-key")
    cache_key.add_argument("--github-output", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        manifest = load_manifest(args.manifest)
        if args.command == "verify":
            _print_inventory(verify_corpus(args.root, manifest))
        elif args.command == "inventory":
            _print_inventory(inventory_corpus(args.root, manifest.include_directories))
        elif args.command == "archive-inventory":
            _print_inventory(inventory_archive(args.archive, manifest))
        elif args.command == "provision":
            _print_inventory(provision_corpus(manifest, args.root, args.cache))
        elif args.command == "cache-key":
            key = manifest_cache_key(args.manifest, manifest)
            print(f"key={key}" if args.github_output else key)
        return 0
    except (BuildSupportError, OSError, tarfile.TarError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
