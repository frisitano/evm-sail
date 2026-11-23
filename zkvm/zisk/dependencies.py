"""Prepare pinned upstream ZisK artifacts separately from guest compilation."""

import argparse
import hashlib
import json
import os
import subprocess
import tempfile
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parent
TARGET = "riscv64ima-zisk-zkvm-elf"
LINKER_SHA256 = "b6746bc7ddf7f48c56ba938350bf91a4ebbf68521d4400e7486c08bb0008670c"


def pin():
    dependency = tomllib.loads((HERE / "Cargo.toml").read_text())["dependencies"]["ziskos"]
    lock = tomllib.loads((HERE / "Cargo.lock").read_text())
    version = next(package["version"] for package in lock["package"] if package["name"] == "ziskos")
    return dependency["git"], dependency["rev"], version


def digest(path):
    with path.open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def git(source, *args):
    return subprocess.check_output(
        ["git", "--no-replace-objects", "-c", "core.hooksPath=/dev/null", "-C", str(source), *args],
        text=True,
    ).strip()


def fetch_source(destination):
    url, revision, _ = pin()
    # Existing checkouts are verified below, never reset or switched.
    if not destination.exists():
        destination.mkdir(parents=True)
        git(destination, "init")
        git(destination, "remote", "add", "origin", url)
        git(destination, "fetch", "--depth=1", "origin", revision)
        git(destination, "checkout", "--detach", revision)
    return destination


def verify_source(source):
    url, revision, _ = pin()
    if git(source, "rev-parse", "HEAD") != revision:
        raise ValueError("ZisK source does not match the pinned release revision")
    if git(source, "diff", "--name-only", "HEAD", "--"):
        raise ValueError("ZisK source has modified tracked release files")
    linker = source / "ziskbuild/zisk_linker_script.ld"
    if linker.is_symlink() or digest(linker) != LINKER_SHA256:
        raise ValueError("ZisK upstream linker checksum mismatch")
    return {
        "repository": url,
        "revision": revision,
        "path": str(source),
        "origin": git(source, "remote", "get-url", "origin"),
    }


def compiler_info():
    toolchain = os.environ.get("ZISK_TOOLCHAIN", "zisk")
    compiler = (
        [os.environ["RUSTC"]] if os.environ.get("RUSTC") else ["rustup", "run", toolchain, "rustc"]
    )
    version = subprocess.check_output([*compiler, "-vV"], text=True)
    if "release: 1.99.0-dev" not in version or "LLVM version: 23.1.1" not in version:
        raise ValueError("ZisK v1.4.0-alpha requires the zisk-4.1.0 custom Rust toolchain")
    cargo = subprocess.check_output(["cargo", f"+{toolchain}", "--version"], text=True)
    return {"command": compiler, "version": version, "cargo": cargo}


def record_artifacts(source, manifest, make):
    source, manifest = source.resolve(), manifest.resolve()
    source_info = verify_source(source)
    artifacts = {
        "staticlib": source / "target" / TARGET / "release/libziskos_staticlib.a",
        "linker": source / "ziskbuild/zisk_linker_script.ld",
    }
    record = {
        "schema_version": 1,
        "source": source_info,
        "version": pin()[2],
        "target": TARGET,
        "build_command": [make, "-C", str(source), "libziskos", "FEATURES="],
        "compiler": compiler_info(),
        **{name: {"path": str(path), "sha256": digest(path)} for name, path in artifacts.items()},
    }
    manifest.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode="w", dir=manifest.parent, delete=False) as output:
        json.dump(record, output, indent=2)
        output.write("\n")
    os.replace(output.name, manifest)
    print(manifest)


def read_artifacts(manifest):
    if not manifest.is_file():
        raise ValueError("ZisK dependencies are not prepared; run make zisk-deps first")
    record = json.loads(manifest.read_text())
    url, revision, version = pin()
    if (
        record["schema_version"] != 1
        or record["source"]["repository"] != url
        or record["source"]["revision"] != revision
        or record["version"] != version
        or record["target"] != TARGET
    ):
        raise ValueError(
            "Prepared ZisK identity differs from the pin; rerun dependency preparation"
        )
    for name in ("staticlib", "linker"):
        artifact = record[name]
        path = Path(artifact["path"])
        if not path.is_absolute() or path.is_symlink() or digest(path) != artifact["sha256"]:
            raise ValueError(f"Prepared ZisK {name} changed; rerun dependency preparation")
    if record["linker"]["sha256"] != LINKER_SHA256:
        raise ValueError("Prepared linker does not match the pinned upstream bytes")
    return record


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    source = commands.add_parser("source", help="check tools and fetch/verify pinned source")
    source.add_argument("directory", type=Path)
    record = commands.add_parser("record", help="record artifacts after the upstream Make build")
    record.add_argument("source", type=Path)
    record.add_argument("manifest", type=Path)
    record.add_argument("--make", default="make")
    path = commands.add_parser("path", help="verify prepared inputs and print one artifact path")
    path.add_argument("manifest", type=Path)
    path.add_argument("artifact", choices=("staticlib", "linker"))
    args = parser.parse_args()
    if args.command == "source":
        compiler_info()
        print(json.dumps(verify_source(fetch_source(args.directory.resolve())), sort_keys=True))
    elif args.command == "record":
        record_artifacts(args.source, args.manifest, args.make)
    else:
        print(read_artifacts(args.manifest)[args.artifact]["path"])


if __name__ == "__main__":
    try:
        main()
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        raise SystemExit(str(error)) from error
