"""Verify frozen import decisions and local receipts, using only the stdlib.

--source-repository is optional research verification, never a build dependency.
No command copies source bytes, creates receipts, or edits the manifest.
"""
import argparse
import copy
import hashlib
import io
import json
import re
import subprocess
import sys
from pathlib import Path, PurePosixPath


def require(condition, message):
    if not condition:
        raise ValueError(message)


def safe_path(value):
    require(isinstance(value, str) and value, "empty path")
    path = PurePosixPath(value)
    require(not path.is_absolute() and not any(p in {".", ".."} for p in value.split("/")),
            "unsafe relative path")
    require(not any(c in value for c in '\\:\0') and str(path) == value, "noncanonical path")
    require(all(part.rstrip(" .") == part for part in path.parts), "Windows path alias")
    return value


def hex_value(value, length):
    return isinstance(value, str) and re.fullmatch(r"[0-9a-f]{%d}" % length, value) is not None


def third_party_entries(manifest, files):
    require(manifest["schema"] == 1 and
            manifest["source_repository"] == "https://github.com/TsudaKageyu/minhook.git" and
            manifest["source_commit"] == "c3fcafdc10146beb5919319d0683e44e3c30d537" and
            manifest["source_tag"] == "v1.3.4", "unreviewed MinHook identity")
    license_path = "src/ntw16s-dll/injection/minhook/LICENSE.txt"
    require(manifest["license_destination"] == license_path, "missing MinHook license identity")
    entries, destinations = {}, set()
    require(len(manifest["entries"]) == 14, "MinHook coverage changed")
    for entry in manifest["entries"]:
        source, destination = safe_path(entry["source"]), safe_path(entry["destination"])
        require(source.startswith("minhook/") and source not in entries, "duplicate/foreign MinHook source")
        require(destination.startswith("src/ntw16s-dll/injection/") and
                destination.casefold() not in destinations, "duplicate/foreign MinHook destination")
        require(entry["action"] in {"retain", "adapt"} and entry["mode"] == "100644" and
                hex_value(entry["blob"], 40) and hex_value(entry["sha256"], 64) and
                type(entry["size"]) is int and entry["size"] > 0, "invalid MinHook disposition/digest")
        require(all(isinstance(entry[k], str) and entry[k].strip()
                    for k in ("notice", "reason", "receiver")), "missing MinHook review")
        raw = files.get(destination)
        require(raw is not None, "missing MinHook local file")
        if entry["action"] == "retain":
            require(len(raw) == entry["size"] and hashlib.sha256(raw).hexdigest() == entry["sha256"],
                    "retained MinHook bytes changed")
        entries[source] = entry
        destinations.add(destination.casefold())
    require(entries.get("minhook/LICENSE.txt", {}).get("destination") == license_path and
            entries["minhook/LICENSE.txt"]["action"] == "retain", "MinHook license not retained")
    return entries


def validate(manifest, inventory, abi, receipts, files, external=None):
    external = external or {}
    require(manifest["schema"] == 1, "unknown manifest schema")
    pinned = re.search(r"^# source-commit: ([0-9a-f]{40})$", inventory, re.M)
    require(pinned and manifest["source_commit"] == pinned[1], "source commit mismatch")
    require(manifest["git_object_format"] == "sha1", "unknown object format")
    require(manifest["import_status"] in {"not-imported", "partial", "imported"}, "invalid import state")
    entries, destinations = {}, set()
    for entry in manifest["entries"]:
        source = safe_path(entry["source"])
        require(source not in entries, "duplicate source")
        entries[source] = entry
        require(hex_value(entry["blob"], 40) and hex_value(entry["sha256"], 64), "invalid source digest")
        require(type(entry["size"]) is int and entry["size"] >= 0, "invalid source size")
        require(entry["mode"] in {"100644", "100755"}, "invalid source mode")
        require(entry["action"] in {"retain", "adapt", "replace", "exclude", "reference"}, "invalid disposition")
        require(all(isinstance(entry[k], str) and entry[k].strip() for k in ("notice", "reason", "receiver")),
                "missing disposition owner/notice/reason")
        destination = entry["destination"]
        if entry["action"] in {"retain", "adapt", "replace"}:
            safe_path(destination)
            require(destination.startswith(("src/", "tests/")), "foreign import destination")
            require(destination.casefold() not in {d.casefold() for d in destinations}, "duplicate destination")
            destinations.add(destination)
        else:
            require(destination is None, "excluded/reference source has destination")
    seed_count = 0
    for line in inventory.splitlines():
        if not line or line.startswith("#"):
            continue
        match = re.fullmatch(r"(100[0-7]{3}) blob ([0-9a-f]{40})\s+(\d+)\t(.+)", line)
        require(match is not None, "invalid seed row")
        entry = entries.get(match[4])
        require(entry and (entry["mode"], entry["blob"], entry["size"]) ==
                (match[1], match[2], int(match[3])), "seed missing or mismatched")
        seed_count += 1
    require(manifest["seed_count"] == seed_count == 105 and len(entries) == 124, "frozen coverage changed")
    restricted = {"frame_buttons.h", "control_glyphs.h", "combo_glyph.h", "menu_glyphs.h", "scroll_glyphs.h"}
    for name in restricted:
        require(entries["src/ntw16s-dll/" + name]["action"] == "retain", "original resource must be retained byte-exact")
    require(len(manifest["includes"]) == 294, "include coverage changed")
    for edge in manifest["includes"]:
        require(edge["source"] in entries and type(edge["line"]) is int and edge["line"] > 0,
                "invalid include owner/location")
        require(edge["include"] and edge["disposition"] in {
            "local-reviewed-seed", "excluded-source-boundary", "excluded-original-native-classifier", "SDK-or-toolchain"
        }, "unresolved include")
        if edge["disposition"] != "SDK-or-toolchain":
            targets = edge["target"] if isinstance(edge["target"], list) else [edge["target"]]
            require(targets and all(t in entries for t in targets), "include target not reviewed")
            if edge["disposition"].startswith("excluded"):
                require(all(entries[t]["action"] == "exclude" for t in targets), "excluded edge would import source")
    require(abi["schema"] == 1 and abi["source_commit"] == manifest["source_commit"] and
            abi["source_abi"] == 2 and abi["standalone_abi"] == 3, "ABI identity mismatch")
    require(len(abi["exports"]) == 34 and len({e["symbol"] for e in abi["exports"]}) == 33,
            "ABI export coverage changed")
    declarations = {}
    for export in abi["exports"]:
        require(export["source_header"] in entries and
                export["source_blob"] == entries[export["source_header"]]["blob"], "ABI source mismatch")
        expected = "private-internal" if export["symbol"] in {
            "Ntw16sBindNativeApi", "Ntw16sBindComboInfoApi"} else "retain-public"
        require(export["decision"] == expected and export["declaration"] and export["reason"], "ABI decision mismatch")
        if export["symbol"] in declarations:
            require(declarations[export["symbol"]] == (export["declaration"], export["decision"]),
                    "conflicting duplicate ABI declaration")
        declarations[export["symbol"]] = (export["declaration"], export["decision"])
    require(abi["helper"]["ordinal"] == 1 and abi["helper"]["noname"] is True and
            abi["helper"]["symbol"] == "Ntw16sFinishHelper", "helper ABI mismatch")
    require(receipts["schema"] == 1, "invalid receipts schema")
    external_destinations = {e["destination"] for e in external.values()}
    seen = set()
    for receipt in receipts["files"]:
        path = safe_path(receipt["path"])
        require(path.startswith(("src/", "tests/")) and path.casefold() not in {p.casefold() for p in seen}, "invalid/duplicate receipt path")
        seen.add(path)
        require(all(isinstance(receipt[k], str) and receipt[k].strip() for k in ("review", "notice")),
                "receipt missing review/notice")
        raw = files.get(path)
        require(raw is not None and hashlib.sha256(raw).hexdigest() == receipt["sha256"], "local content digest mismatch")
        if receipt["source"] is None:
            require(receipt["source_blob"] is None and receipt["disposition"] == "authored", "invalid authored receipt")
            require(path not in destinations | external_destinations, "authored receipt bypasses source disposition")
            continue
        entry = entries.get(receipt["source"]) or external.get(receipt["source"])
        require(entry and path == entry["destination"] and receipt["source_blob"] == entry["blob"] and
                receipt["disposition"] == entry["action"], "receipt/source disposition mismatch")
        if entry["action"] == "retain":
            require(receipt["sha256"] == entry["sha256"], "retained source is not byte exact")
        if entry["action"] == "replace":
            require(receipt["sha256"] != entry["sha256"], "restricted source bytes copied")
    require(all(path in seen for path in files if path.startswith("src/") or path in destinations),
            "unreceipted local import")
    if manifest["import_status"] == "not-imported":
        require(not receipts["files"] and not files, "files present before import admission")
    if manifest["import_status"] == "imported":
        require(destinations <= seen, "incomplete final import")
    require(all(e["destination"] in seen for e in external.values()), "missing third-party receipt")
    for path in seen:
        if "/injection/detours/" in path and not path.endswith("LICENSE.md"):
            require("src/ntw16s-dll/injection/detours/LICENSE.md" in seen, "Detours license missing")
    return len(entries), len(manifest["includes"]), len(seen)


def check_source(manifest, repository, prefix=""):
    def git(*args, data=None):
        return subprocess.check_output(["git", "-C", str(repository), *args], input=data, timeout=30)
    tree = {}
    for line in git("ls-tree", "-r", "-l", manifest["source_commit"]).decode().splitlines():
        metadata, path = line.split("\t", 1)
        tree[path] = metadata.split()
    entries = manifest["entries"]
    stream = io.BytesIO(git("cat-file", "--batch", data="".join(e["blob"] + "\n" for e in entries).encode()))
    for entry in entries:
        require(tree.get(entry["source"].removeprefix(prefix)) == [entry["mode"], "blob", entry["blob"], str(entry["size"])],
                "fixed source tree differs")
        blob, kind, size = stream.readline().decode().split()
        raw = stream.read(int(size))
        require(stream.read(1) == b"\n" and blob == entry["blob"] and kind == "blob" and
                int(size) == entry["size"] and hashlib.sha256(raw).hexdigest() == entry["sha256"],
                "fixed source bytes differ")
    print("Source research PASS: %d/%d fixed tree/blob/size/SHA-256" % (len(entries), len(entries)))


def self_test(manifest, inventory, abi, local_files):
    cases = {
        "floating-source": lambda m, a: m.update(source_commit="0" * 40),
        "missing-seed": lambda m, a: m["entries"].pop(0),
        "duplicate-source": lambda m, a: m["entries"].append(copy.deepcopy(m["entries"][0])),
        "escaping-source": lambda m, a: m["entries"][0].update(source="../escape"),
        "empty-receiver": lambda m, a: m["entries"][0].update(receiver=""),
        "malformed-digest": lambda m, a: m["entries"][0].update(sha256="bad"),
        "wrong-seed-blob": lambda m, a: m["entries"][0].update(blob="0" * 40),
        "unreviewed-include": lambda m, a: m["includes"][0].update(target="missing.h", disposition="local-reviewed-seed"),
        "lost-include": lambda m, a: m["includes"].pop(),
        "wrong-ABI": lambda m, a: a.update(standalone_abi=2),
        "lost-export": lambda m, a: a["exports"].pop(),
        "wrong-helper": lambda m, a: a["helper"].update(ordinal=2),
        "substituted-resource": lambda m, a: next(e for e in m["entries"] if e["source"].endswith("frame_buttons.h")).update(action="replace"),
        "case-alias-destination": lambda m, a: next(e for e in m["entries"] if e["source"].endswith("control_glyphs.h")).update(
            destination="src/ntw16s-dll/COMBO_GLYPH.H"),
        "excluded-edge-import": lambda m, a: next(e for e in m["entries"] if e["source"] == "src/common/rpc/local_binding.h").update(
            action="adapt", destination="src/ntw16s-dll/forbidden.h"),
        "conflicting-ABI-declaration": lambda m, a: next(e for e in a["exports"] if e["source_header"].endswith("theme.h")).update(
            declaration="conflicting declaration"),
    }
    for name, mutate in cases.items():
        m, a = copy.deepcopy(manifest), copy.deepcopy(abi)
        mutate(m, a)
        try:
            validate(m, inventory, a, {"schema": 1, "files": []}, {})
        except (ValueError, KeyError, TypeError):
            continue
        raise ValueError("negative fixture was accepted: " + name)
    m = copy.deepcopy(manifest)
    m["import_status"] = "partial"
    entry = next(e for e in m["entries"] if e["source"].endswith("frame_buttons.h"))
    raw = local_files[entry["destination"]]
    receipt = {"path": entry["destination"], "source": entry["source"], "source_blob": entry["blob"],
               "disposition": "retain", "sha256": hashlib.sha256(raw).hexdigest(),
               "review": "byte-exact source fixture", "notice": "original provenance retained"}
    receipts = {"schema": 1, "files": [receipt]}
    files = {entry["destination"]: raw}
    validate(m, inventory, abi, receipts, files)
    import_cases = ("tampered-local", "substitution-with-updated-receipt", "missing-receipt", "unsafe-destination",
                    "duplicate-receipt", "empty-notice", "missing-Detours-license")
    for name in import_cases:
        mutated_m, mutated_r, mutated_f = copy.deepcopy(m), copy.deepcopy(receipts), dict(files)
        if name == "tampered-local":
            mutated_f[entry["destination"]] = b"changed"
        elif name == "substitution-with-updated-receipt":
            mutated_f[entry["destination"]] = b"authored replacement"
            mutated_r["files"][0]["sha256"] = hashlib.sha256(mutated_f[entry["destination"]]).hexdigest()
        elif name == "missing-receipt":
            mutated_r["files"] = []
        elif name == "unsafe-destination":
            mutated_r["files"][0]["path"] = "src/../outside.c"
        elif name == "duplicate-receipt":
            mutated_r["files"].append(copy.deepcopy(receipt))
        elif name == "empty-notice":
            mutated_r["files"][0]["notice"] = ""
        else:
            detours = next(e for e in mutated_m["entries"] if e["source"].endswith("detours/creatwth.cpp"))
            # Retired in the actual tree; this isolated fixture still admits a
            # real source destination so rejection specifically needs a license.
            detours.update(action="adapt", destination="src/ntw16s-dll/injection/detours/creatwth.cpp")
            new_receipt = dict(receipt, path=detours["destination"], source=detours["source"],
                               source_blob=detours["blob"], disposition="adapt", notice="MIT-Detours")
            mutated_r["files"].append(new_receipt)
            mutated_f[detours["destination"]] = raw
        try:
            validate(mutated_m, inventory, abi, mutated_r, mutated_f)
        except (ValueError, KeyError, TypeError):
            continue
        raise ValueError("import fixture was accepted: " + name)
    print("Source SelfTest PASS: %d rejection cases and one byte-exact resource receipt" % (len(cases) + len(import_cases)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--source-repository", type=Path)
    parser.add_argument("--third-party-repository", type=Path)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    folder = root / "docs/etc/import"
    manifest = json.loads((folder / "source-manifest.json").read_text(encoding="utf-8-sig"))
    abi = json.loads((folder / "abi-decisions.json").read_text(encoding="utf-8-sig"))
    inventory = (folder / "source-inventory.tsv").read_text(encoding="utf-8-sig")
    receipt_path = folder / "import-receipts.json"
    receipts = json.loads(receipt_path.read_text(encoding="utf-8-sig")) if receipt_path.exists() else {"schema": 1, "files": []}
    candidates = {e["destination"] for e in manifest["entries"] if e["destination"] is not None}
    candidates.update(r["path"] for r in receipts["files"])
    source_root = root / "src"
    require(not source_root.is_symlink(), "symlink source root")
    if source_root.exists():
        candidates.update(p.relative_to(root).as_posix() for p in source_root.rglob("*") if p.is_file())
    files = {}
    for candidate in candidates:
        safe_path(candidate)
        path = root / candidate
        require(path.resolve().is_relative_to(root), "import escapes root via reparse/symlink")
        if path.is_file():
            files[candidate] = path.read_bytes()
    third_party_path = folder / "minhook-manifest.json"
    third_party = json.loads(third_party_path.read_text(encoding="utf-8")) if third_party_path.exists() else None
    external = third_party_entries(third_party, files) if third_party else {}
    counts = validate(manifest, inventory, abi, receipts, files, external)
    print("Source manifest PASS: %d decisions / %d includes / %d local imports" % counts)
    if args.self_test:
        self_test(manifest, inventory, abi, files)
        if third_party:
            mutations = {
                "floating-third-party": lambda m, f: m.update(source_commit="0" * 40),
                "lost-third-party": lambda m, f: m["entries"].pop(),
                "duplicate-third-party": lambda m, f: m["entries"].__setitem__(1, copy.deepcopy(m["entries"][0])),
                "third-party-path-escape": lambda m, f: m["entries"][0].update(destination="src/../escape"),
                "third-party-empty-review": lambda m, f: m["entries"][0].update(notice=""),
                "third-party-license-omitted": lambda m, f: f.pop(m["license_destination"]),
                "third-party-license-modified": lambda m, f: f.update({m["license_destination"]: b"changed"}),
            }
            for name, mutate in mutations.items():
                m, f = copy.deepcopy(third_party), dict(files)
                mutate(m, f)
                try:
                    third_party_entries(m, f)
                except (ValueError, KeyError, TypeError):
                    continue
                raise ValueError("third-party fixture accepted: " + name)
            for mode in ("authored-bypass", "missing-receipt"):
                mutated = copy.deepcopy(receipts)
                receipt = next(r for r in mutated["files"] if r["source"] in external)
                if mode == "authored-bypass":
                    receipt.update(source=None, source_blob=None, disposition="authored")
                else:
                    mutated["files"].remove(receipt)
                try:
                    validate(manifest, inventory, abi, mutated, files, external)
                except (ValueError, KeyError, TypeError):
                    continue
                raise ValueError("third-party receipt fixture accepted: " + mode)
            print("Third-party SelfTest PASS: 9 rejection cases; 14 MinHook source decisions")
    if args.source_repository:
        check_source(manifest, args.source_repository)
    if args.third_party_repository:
        require(third_party is not None, "no third-party manifest")
        check_source(third_party, args.third_party_repository, "minhook/")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, KeyError, TypeError, OSError, subprocess.SubprocessError) as error:
        print("Source manifest FAIL:", error, file=sys.stderr)
        sys.exit(1)
