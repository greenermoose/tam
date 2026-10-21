#!/usr/bin/env python3
"""dev.py: Interactive developer loop for committing, testing, and installing tam edits.

Runs the ten-step workflow defined in docs/plans/dev-loop.md (TCK11):
  1. Preflight
  2. Claim
  3. Collect changes
  4. Versions
  5. Build and test
  6. Commit
  7. Index follow-up
  8. Install
  9. Push
 10. Finish
"""

import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import signal
import subprocess
import sys

GENERATED_EXACT = frozenset({
    "knowledge/index.tsv",
    "knowledge/index.json",
    "knowledge/index.parents.tsv",
    "knowledge/index.kinds.tsv",
    "knowledge/index.skills.tsv",
    "knowledge/coverage.json",
    "knowledge/.build_id",
})
INDEX_PATHS = (
    "knowledge/index.tsv",
    "knowledge/index.json",
    "knowledge/index.parents.tsv",
    "knowledge/index.kinds.tsv",
    "knowledge/index.skills.tsv",
    "knowledge/coverage.json",
)
CATEGORIES = (
    "you edit",
    "script edits",
    "generated, never edit",
    "history, read-only",
)


class DevError(Exception):
    """Stops the developer loop with a plain error message."""


def clean_make_env(extra=None):
    env = dict(os.environ)
    for key in ("MAKEFLAGS", "MFLAGS", "MAKELEVEL"):
        env.pop(key, None)
    if extra:
        env.update(extra)
    return env


def run_git(root, *args, check=True):
    res = subprocess.run(
        ["git", "-C", str(root), *args],
        capture_output=True,
        text=True,
        check=False,
    )
    if check and res.returncode != 0:
        detail = (res.stderr or res.stdout).strip()
        raise DevError(f"git {' '.join(args)} failed: {detail}")
    return res


def run_make(root, *args):
    return subprocess.run(
        ["make", "-C", str(root), "--no-print-directory", *args],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        env=clean_make_env(),
        check=False,
    )


def is_generated_path(rel_path):
    if rel_path in GENERATED_EXACT:
        return True
    p = PurePosixPath(rel_path)
    if rel_path == "build" or rel_path.startswith("build/"):
        return True
    if "__pycache__" in p.parts:
        return True
    return False


def scope_covers_path(repo_scope, rel_path):
    """Return True if rel_path is at or under repo_scope (a 'tam/...' or 'tam' scope)."""
    if repo_scope == "tam":
        return True
    if repo_scope.startswith("tam/"):
        rel_scope = repo_scope[4:].rstrip("/")
    else:
        rel_scope = repo_scope.rstrip("/")
    if not rel_scope:
        return True
    path_obj = PurePosixPath(rel_path)
    scope_obj = PurePosixPath(rel_scope)
    return path_obj == scope_obj or scope_obj in path_obj.parents


def load_repo_map(root):
    """Load optional path -> category mapping from docs/repo-map.md (added in TCK12)."""
    map_file = root / "docs" / "repo-map.md"
    if not map_file.is_file():
        return {}
    mapping = {}
    text = map_file.read_text(encoding="utf-8")
    for line in text.splitlines():
        code_matches = re.findall(r"`([^`]+)`", line)
        if not code_matches:
            continue
        found_cat = None
        lower = line.lower()
        for cat in CATEGORIES:
            if cat in lower:
                found_cat = cat
                break
        if found_cat:
            entry = code_matches[0].strip()
            mapping[entry] = found_cat
    return mapping


def categorize_path(rel_path, repo_map):
    if not repo_map:
        return None
    best_match = None
    best_len = -1
    p = PurePosixPath(rel_path)
    for entry, cat in repo_map.items():
        clean = entry.rstrip("/")
        entry_p = PurePosixPath(clean)
        if p == entry_p or entry_p in p.parents:
            if len(clean) > best_len:
                best_len = len(clean)
                best_match = cat
    return best_match


def scan_working_tree(root, owner, session):
    """Parse git status and active tam-work claims; return grouped and excluded paths."""
    claims_res = subprocess.run(
        ["tam-work", "status", "--json", "--repo", "tam"],
        cwd=str(root),
        capture_output=True,
        text=True,
        check=False,
    )
    if claims_res.returncode != 0:
        detail = (claims_res.stderr or claims_res.stdout).strip()
        raise DevError(f"tam-work status failed: {detail}")
    try:
        all_claims = json.loads(claims_res.stdout or "[]")
    except json.JSONDecodeError as exc:
        raise DevError(f"invalid JSON from tam-work status: {exc}") from exc

    other_claims = [
        row for row in all_claims
        if row.get("status") != "released"
        and not (row.get("owner") == owner and row.get("session") == session)
    ]

    status_res = run_git(root, "status", "--porcelain=v1", "-z", "--untracked-files=all")
    tokens = status_res.stdout.split("\0")
    raw_entries = []
    idx = 0
    while idx < len(tokens):
        entry = tokens[idx]
        idx += 1
        if len(entry) < 4:
            continue
        xy = entry[:2]
        path = entry[3:].rstrip("/")
        orig_path = None
        if xy[0] in ("R", "C") and idx < len(tokens):
            orig_path = tokens[idx].rstrip("/")
            idx += 1
        raw_entries.append((xy, path, orig_path))

    paths = []
    untracked = set()
    excluded = []
    seen = set()

    for xy, path, orig_path in raw_entries:
        if is_generated_path(path):
            continue
        matching_claim = None
        for row in other_claims:
            if any(scope_covers_path(s, path) for s in row.get("scopes", [])):
                matching_claim = row
                break
        if matching_claim is not None:
            excluded.append((path, matching_claim))
            continue
        if path not in seen:
            seen.add(path)
            paths.append(path)
        if xy == "??":
            untracked.add(path)
        if orig_path and not is_generated_path(orig_path) and orig_path not in seen:
            if not any(
                any(scope_covers_path(s, orig_path) for s in row.get("scopes", []))
                for row in other_claims
            ):
                seen.add(orig_path)
                paths.append(orig_path)

    corpus = [p for p in paths if p.startswith("knowledge/")]
    command = [p for p in paths if p.startswith("src/")]
    other = [p for p in paths if not p.startswith("knowledge/") and not p.startswith("src/")]

    return {
        "paths": paths,
        "untracked": untracked,
        "corpus": corpus,
        "command": command,
        "other": other,
        "excluded": excluded,
    }


def replace_once(path, pattern, replacement, regex=False):
    """Replace pattern with replacement in path, failing unless pattern occurs exactly once."""
    text = path.read_text(encoding="utf-8")
    if regex:
        matches = list(re.finditer(pattern, text, flags=re.M))
        if len(matches) != 1:
            raise DevError(
                f"{path.name}: expected pattern {pattern!r} exactly once, found {len(matches)}"
            )
        new_text = re.sub(pattern, replacement, text, count=1, flags=re.M)
    else:
        count = text.count(pattern)
        if count != 1:
            raise DevError(
                f"{path.name}: expected {pattern!r} exactly once, found {count}"
            )
        new_text = text.replace(pattern, replacement, 1)
    path.write_text(new_text, encoding="utf-8")


# ---------------------------------------------------------------------------
# 1. Preflight
# ---------------------------------------------------------------------------

def step_preflight(root, dry_run=False):
    print("== 1. Preflight ==")
    if not dry_run and not (sys.stdin.isatty() and sys.stdout.isatty()):
        raise DevError("stdin and stdout must be terminals.")

    branch_res = run_git(root, "symbolic-ref", "--quiet", "--short", "HEAD", check=False)
    branch = branch_res.stdout.strip() if branch_res.returncode == 0 else ""
    if branch != "develop":
        raise DevError(
            f"must be on branch 'develop' (current: {branch or 'detached HEAD'})."
        )

    git_dir_res = run_git(root, "rev-parse", "--git-dir")
    git_dir = Path(git_dir_res.stdout.strip())
    if not git_dir.is_absolute():
        git_dir = (root / git_dir).resolve()

    in_progress = [
        ("merge", git_dir / "MERGE_HEAD"),
        ("rebase", git_dir / "rebase-merge"),
        ("rebase", git_dir / "rebase-apply"),
        ("cherry-pick", git_dir / "CHERRY_PICK_HEAD"),
    ]
    for op_name, marker in in_progress:
        if marker.exists():
            raise DevError(f"{op_name} in progress ({marker.name} exists).")

    libtam_res = run_make(root, "libtam-check")
    if libtam_res.returncode != 0:
        raise DevError(f"make libtam-check failed:\n{libtam_res.stdout.strip()}")

    if shutil.which("tam-work") is None:
        raise DevError("tam-work not found on PATH.")
    print("Preflight ok: on branch develop, libtam and tam-work ready.")


# ---------------------------------------------------------------------------
# 2. Claim
# ---------------------------------------------------------------------------

def step_claim(root, paths, claim_state, print_heading=True):
    if print_heading:
        print("== 2. Claim ==")
    os.environ["TAM_WORK_OWNER"] = claim_state["owner"]
    os.environ["TAM_WORK_SESSION"] = claim_state["session"]
    if not paths:
        return
    scope_args = []
    for p in paths:
        scope_args.extend(["--scope", p])
    res = subprocess.run(
        ["tam-work", "begin", "--allow-dirty", *scope_args, "--note", "make dev"],
        cwd=str(root),
        capture_output=True,
        text=True,
        check=False,
    )
    if res.returncode != 0:
        detail = (res.stderr or res.stdout).strip()
        raise DevError(f"tam-work begin failed: {detail}")
    claim_state["active"] = True
    if print_heading:
        first_line = res.stdout.strip().splitlines()[0] if res.stdout.strip() else "Claim active."
        print(first_line)


def release_claim(root, claim_state, note="make dev finished"):
    if not claim_state.get("active"):
        return
    claim_state["active"] = False
    os.environ["TAM_WORK_OWNER"] = claim_state["owner"]
    os.environ["TAM_WORK_SESSION"] = claim_state["session"]
    subprocess.run(
        ["tam-work", "finish", "--note", note],
        cwd=str(root),
        capture_output=True,
        text=True,
        check=False,
    )


# ---------------------------------------------------------------------------
# 3. Collect changes
# ---------------------------------------------------------------------------

def step_collect_changes(root, claim_state, dry_run=False, changes=None):
    print("== 3. Collect changes ==")
    if changes is None:
        changes = scan_working_tree(root, claim_state["owner"], claim_state["session"])

    repo_map = load_repo_map(root)
    warnings = []

    def format_entry(p):
        cat = categorize_path(p, repo_map)
        if cat in ("generated, never edit", "history, read-only"):
            warnings.append(f"Warning: {p} is marked '{cat}' in docs/repo-map.md")
        tag = " (untracked)" if p in changes["untracked"] else ""
        cat_label = f" [{cat}]" if cat else ""
        return f"  {p}{tag}{cat_label}"

    for group_title, key in (
        ("Corpus (knowledge/)", "corpus"),
        ("Command (src/)", "command"),
        ("Other", "other"),
    ):
        items = changes[key]
        if items:
            print(f"{group_title}:")
            for p in items:
                print(format_entry(p))

    if changes["excluded"]:
        print("Excluded paths (claimed by another session):")
        for path, row in changes["excluded"]:
            cid = row.get("id", "unknown")
            owner = row.get("owner", "unknown")
            scopes = ", ".join(row.get("scopes", []))
            print(f"  {path} (excluded by {cid}, owner {owner}, scope {scopes})")

    for w in warnings:
        print(w)

    if not changes["paths"]:
        print("Nothing to commit.")
        return None

    tracked = [p for p in changes["paths"] if p not in changes["untracked"]]
    if tracked:
        stat_res = run_git(root, "diff", "--stat", "HEAD", "--", *tracked, check=False)
        if stat_res.stdout.strip():
            print(stat_res.stdout.rstrip())
    for p in sorted(changes["untracked"]):
        print(f" {p} | (new untracked file)")

    if dry_run:
        return changes

    while True:
        try:
            ans = input("[c]ontinue, [d]iff, [q]uit [c]: ").strip().lower()
        except EOFError as exc:
            raise DevError("unexpected EOF while reading prompt.") from exc
        if ans in ("", "c", "continue"):
            return changes
        if ans in ("q", "quit"):
            print("Quit. Nothing was committed.")
            return None
        if ans in ("d", "diff"):
            diff_chunks = []
            if tracked:
                d_res = run_git(root, "diff", "HEAD", "--", *tracked, check=False)
                if d_res.stdout:
                    diff_chunks.append(d_res.stdout)
            for p in sorted(changes["untracked"]):
                u_res = run_git(root, "diff", "--no-index", "--", "/dev/null", p, check=False)
                if u_res.stdout:
                    diff_chunks.append(u_res.stdout)
            pager = os.environ.get("PAGER") or "less"
            subprocess.run(pager, shell=True, input="".join(diff_chunks), text=True, check=False)


# ---------------------------------------------------------------------------
# 4. Versions
# ---------------------------------------------------------------------------

def parse_semver(ver_str, label):
    m = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", ver_str.strip())
    if not m:
        raise DevError(f"invalid {label} version {ver_str!r} (expected X.Y.Z)")
    return int(m.group(1)), int(m.group(2)), int(m.group(3))


def next_versions(ver_str, label):
    major, minor, patch = parse_semver(ver_str, label)
    return f"{major}.{minor}.{patch + 1}", f"{major}.{minor + 1}.0"


def read_command_version_from_text(text, source_label="src/version.h"):
    m = re.search(r'^#define\s+TAM_COMMAND_VERSION\s+"([^"]+)"', text, flags=re.M)
    if not m:
        raise DevError(f"{source_label}: missing TAM_COMMAND_VERSION definition")
    return m.group(1).strip()


def read_corpus_version(root, from_head=False):
    if from_head:
        res = run_git(root, "show", "HEAD:knowledge/version")
        return res.stdout.strip()
    path = root / "knowledge" / "version"
    if not path.is_file():
        raise DevError("knowledge/version not found")
    return path.read_text(encoding="utf-8").strip()


def read_command_version(root, from_head=False):
    if from_head:
        res = run_git(root, "show", "HEAD:src/version.h")
        return read_command_version_from_text(res.stdout, "HEAD:src/version.h")
    path = root / "src" / "version.h"
    if not path.is_file():
        raise DevError("src/version.h not found")
    return read_command_version_from_text(path.read_text(encoding="utf-8"), "src/version.h")


def recover_existing_summary(root, ver_str, kind):
    roadmap = root / "ROADMAP.md"
    if not roadmap.is_file():
        return None
    text = roadmap.read_text(encoding="utf-8")
    pattern = r"^\|\s*\*\*" + re.escape(ver_str) + r"\s+" + re.escape(kind) + r"\*\*\s*\|\s*([^|]+?)\s*\|"
    m = re.search(pattern, text, flags=re.M)
    return m.group(1).strip() if m else None


def prompt_bump_choice(lane_label, current_ver, patch_ver, minor_ver):
    while True:
        prompt = (
            f"{lane_label} version ({current_ver} -> patch {patch_ver} or minor {minor_ver}) "
            f"[p]atch (default) / [m]inor: "
        )
        ans = input(prompt).strip().lower()
        if ans in ("", "p", "patch"):
            return "patch", patch_ver
        if ans in ("m", "minor"):
            return "minor", minor_ver
        print("Please answer 'p' (patch) or 'm' (minor).")


def apply_corpus_version_edits(root, old_ver, new_ver, summary, today):
    cmd_ver = read_command_version(root, from_head=False)
    version_file = root / "knowledge" / "version"
    replace_once(version_file, old_ver, new_ver)

    roadmap = root / "ROADMAP.md"
    replace_once(
        roadmap,
        f"**Current Corpus Version:** `{old_ver}`.",
        f"**Current Corpus Version:** `{new_ver}`.",
    )
    replace_once(
        roadmap,
        f"| `develop` | {cmd_ver} / {old_ver} |",
        f"| `develop` | {cmd_ver} / {new_ver} |",
    )
    new_row = f"| **{new_ver} corpus** | {summary} | Done ({today}) | {summary} |\n"
    replace_once(
        roadmap,
        r"(\|[^\n]+\|\n)(\n+## 3\. Current state and follow-up\n)",
        r"\1" + new_row.replace("\\", "\\\\") + r"\2",
        regex=True,
    )
    return ["knowledge/version", "ROADMAP.md"]


def apply_command_version_edits(root, old_ver, new_ver, bump_kind, summary, today):
    corpus_ver = read_corpus_version(root, from_head=False)
    version_h = root / "src" / "version.h"
    replace_once(
        version_h,
        r'^(#define\s+TAM_COMMAND_VERSION\s+")' + re.escape(old_ver) + r'(")$',
        r"\g<1>" + new_ver + r"\g<2>",
        regex=True,
    )

    run_sh = root / "tests" / "run.sh"
    replace_once(
        run_sh,
        f"^tam command {old_ver}",
        f"^tam command {new_ver}",
    )

    roadmap = root / "ROADMAP.md"
    replace_once(
        roadmap,
        f"**Current Command Version:** `{old_ver}`.",
        f"**Current Command Version:** `{new_ver}`.",
    )
    replace_once(
        roadmap,
        f"| `develop` | {old_ver} / {corpus_ver} |",
        f"| `develop` | {new_ver} / {corpus_ver} |",
    )
    new_row = f"| **{new_ver} command** | {summary} | Done ({today}) | {summary} |\n"
    replace_once(
        roadmap,
        r"(\|[^\n]+\|\n)(\n+## 3\. Current state and follow-up\n)",
        r"\1" + new_row.replace("\\", "\\\\") + r"\2",
        regex=True,
    )

    dev_md = root / "docs" / "plans" / "development.md"
    entry = f"- **{today}:** Command {new_ver} ({bump_kind}): {summary}\n"
    replace_once(
        dev_md,
        r"(^## Decision record\n+(?:[^-\n][^\n]*\n+)*)(- \*\*)",
        r"\1" + entry.replace("\\", "\\\\") + r"\2",
        regex=True,
    )
    return ["src/version.h", "tests/run.sh", "ROADMAP.md", "docs/plans/development.md"]


def step_versions(root, changes, claim_state, dry_run=False, today=None):
    print("== 4. Versions ==")
    today = today or datetime.now().strftime("%Y-%m-%d")
    has_corpus = bool(changes["corpus"])
    has_command = bool(changes["command"])

    result = {
        "corpus_version": None,
        "command_version": None,
        "summary": None,
    }

    if not has_corpus and not has_command:
        print("No corpus or command changes; version files unchanged.")
        if not dry_run:
            while not result["summary"]:
                result["summary"] = input("One-line summary: ").strip()
        return result

    need_corpus_bump = False
    corpus_old = corpus_patch = corpus_minor = None
    if has_corpus:
        wt_corpus = read_corpus_version(root, from_head=False)
        head_corpus = read_corpus_version(root, from_head=True)
        if wt_corpus != head_corpus:
            print(f"Corpus: already bumped to {wt_corpus}")
            result["corpus_version"] = wt_corpus
            result["summary"] = recover_existing_summary(root, wt_corpus, "corpus")
        else:
            need_corpus_bump = True
            corpus_old = head_corpus
            corpus_patch, corpus_minor = next_versions(head_corpus, "corpus")
            if dry_run:
                print(
                    f"Corpus version: {head_corpus} -> {corpus_patch} (patch, default) "
                    f"or {corpus_minor} (minor)"
                )

    need_cmd_bump = False
    cmd_old = cmd_patch = cmd_minor = None
    if has_command:
        wt_cmd = read_command_version(root, from_head=False)
        head_cmd = read_command_version(root, from_head=True)
        if wt_cmd != head_cmd:
            print(f"Command: already bumped to {wt_cmd}")
            result["command_version"] = wt_cmd
            if not result["summary"]:
                result["summary"] = recover_existing_summary(root, wt_cmd, "command")
        else:
            need_cmd_bump = True
            cmd_old = head_cmd
            cmd_patch, cmd_minor = next_versions(head_cmd, "command")
            if dry_run:
                print(
                    f"Command version: {head_cmd} -> {cmd_patch} (patch, default) "
                    f"or {cmd_minor} (minor)"
                )

    if dry_run:
        return result

    corpus_choice = None
    if need_corpus_bump:
        corpus_choice = prompt_bump_choice("Corpus", corpus_old, corpus_patch, corpus_minor)
        result["corpus_version"] = corpus_choice[1]

    cmd_choice = None
    if need_cmd_bump:
        cmd_choice = prompt_bump_choice("Command", cmd_old, cmd_patch, cmd_minor)
        result["command_version"] = cmd_choice[1]

    if need_corpus_bump or need_cmd_bump or not result["summary"]:
        while not result["summary"]:
            result["summary"] = input("One-line summary: ").strip()

    edited_files = []
    if need_corpus_bump:
        targets = ["knowledge/version", "ROADMAP.md"]
        step_claim(root, targets, claim_state, print_heading=False)
        edited_files.extend(
            apply_corpus_version_edits(
                root, corpus_old, corpus_choice[1], result["summary"], today
            )
        )

    if need_cmd_bump:
        targets = ["src/version.h", "tests/run.sh", "ROADMAP.md", "docs/plans/development.md"]
        step_claim(root, targets, claim_state, print_heading=False)
        edited_files.extend(
            apply_command_version_edits(
                root, cmd_old, cmd_choice[1], cmd_choice[0], result["summary"], today
            )
        )

    for rel in edited_files:
        if rel not in changes["paths"]:
            changes["paths"].append(rel)
            if rel.startswith("knowledge/"):
                changes["corpus"].append(rel)
            elif rel.startswith("src/"):
                changes["command"].append(rel)
            else:
                changes["other"].append(rel)

    return result


# ---------------------------------------------------------------------------
# 5. Build and test first
# ---------------------------------------------------------------------------

# Checks that read the developer's pages. Any other failing step is a build,
# command or repository test that an agent fixes.
PAGE_CHECKS = (
    "tools/build-index.py",
    "tools/lint-corpus.py",
    "tests/test_lint_corpus.py",
    "tests/test_build_index.py",
    "tests/guide.sh",
)
RECIPE_RE = re.compile(r"^(tests/\S+|python3 |TAM_TEST_SKIP_OLLAMA=)")
PAGE_ONLY_OTHER = frozenset({"ROADMAP.md"})


def failing_check(output):
    """The last test or check recipe make echoed before it stopped."""
    last = None
    for line in output.splitlines():
        if RECIPE_RE.match(line):
            last = line.strip()
    return last


def explain_test_failure(output, changes):
    lines = [line for line in output.splitlines() if line.startswith("FAIL")]
    check = failing_check(output)
    page_check = check is not None and any(c in check for c in PAGE_CHECKS)
    page_only = bool(changes["corpus"]) and not changes["command"] and all(
        rel in PAGE_ONLY_OTHER for rel in changes["other"]
    )
    if check is None:
        lines.append("The build failed before the tests ran.")
    else:
        lines.append(f"Failing check: {check}")
    if page_only and not page_check:
        lines.append(
            "You changed only pages, and this is not a page check: it is a build, "
            "command or repository test. Your pages are probably fine. Ask an agent "
            "to fix the named test, then run make dev again; it keeps your version bump."
        )
    elif page_check:
        lines.append("This check reads the pages. Fix the page it names, then run make dev again.")
    return lines


def step_build_and_test(root, changes):
    print("== 5. Build and test ==")
    res = run_make(root, "test")
    if res.returncode != 0:
        lines = res.stdout.splitlines()
        tail = "\n".join(lines[-40:])
        print(tail)
        print()
        print("\n".join(explain_test_failure(res.stdout, changes)))
        raise DevError("make test failed; version edits remain in the working tree.")
    print("make test passed.")


# ---------------------------------------------------------------------------
# 6. Commit
# ---------------------------------------------------------------------------

def build_default_subject(summary, corpus_version, command_version):
    suffixes = []
    if corpus_version:
        suffixes.append(f"corpus {corpus_version}")
    if command_version:
        suffixes.append(f"command {command_version}")
    if suffixes:
        return f"{summary}; {', '.join(suffixes)}"
    return summary or ""


def step_commit(root, changes, version_info, committed):
    print("== 6. Commit ==")
    default_subject = build_default_subject(
        version_info.get("summary"),
        version_info.get("corpus_version"),
        version_info.get("command_version"),
    )
    if default_subject:
        subject = input(f"Commit subject [{default_subject}]: ").strip() or default_subject
    else:
        subject = ""
        while not subject:
            subject = input("Commit subject: ").strip()

    body = input("Commit body (optional): ").strip()

    untracked_to_add = [p for p in changes["paths"] if p in changes["untracked"]]
    if untracked_to_add:
        run_git(root, "add", "--", *untracked_to_add)

    commit_cmd = ["commit", "-m", subject]
    if body:
        commit_cmd.extend(["-m", body])
    commit_cmd.extend(["--", *changes["paths"]])
    run_git(root, *commit_cmd)

    sha = run_git(root, "rev-parse", "--short", "HEAD").stdout.strip()
    committed.append((sha, subject))
    print(f"Committed {sha}: {subject}")
    return sha


# ---------------------------------------------------------------------------
# 7. Index follow-up
# ---------------------------------------------------------------------------

def step_index_followup(root, main_sha, claim_state, committed):
    print("== 7. Index follow-up ==")
    res = run_make(root, "corpus")
    if res.returncode != 0:
        raise DevError(f"make corpus failed:\n{res.stdout.strip()}")

    st = run_git(root, "status", "--porcelain=v1", "-z", "--", *INDEX_PATHS)
    tokens = st.stdout.split("\0")
    changed_indexes = []
    untracked_indexes = []
    for entry in tokens:
        if len(entry) < 4:
            continue
        xy = entry[:2]
        path = entry[3:].rstrip("/")
        if path in INDEX_PATHS and path not in changed_indexes:
            changed_indexes.append(path)
            if xy == "??":
                untracked_indexes.append(path)

    if changed_indexes:
        step_claim(root, changed_indexes, claim_state, print_heading=False)
        if untracked_indexes:
            run_git(root, "add", "--", *untracked_indexes)
        idx_subject = f"Refresh corpus index revisions after {main_sha}"
        run_git(root, "commit", "-m", idx_subject, "--", *changed_indexes)
        idx_sha = run_git(root, "rev-parse", "--short", "HEAD").stdout.strip()
        committed.append((idx_sha, idx_subject))
        print(f"Committed {idx_sha}: {idx_subject}")

    res2 = run_make(root, "corpus")
    if res2.returncode != 0:
        raise DevError(f"second make corpus failed:\n{res2.stdout.strip()}")


# ---------------------------------------------------------------------------
# 8. Install
# ---------------------------------------------------------------------------

def step_install(root):
    print("== 8. Install ==")
    cmd_res = run_make(root, "command")
    if cmd_res.returncode != 0:
        raise DevError(f"make command failed:\n{cmd_res.stdout.strip()}")

    default_raw = "~/.local/dev"
    default_expanded = os.path.expanduser(default_raw)
    raw_prefix = input(f"Install prefix [{default_raw}]: ").strip() or default_raw
    expanded_prefix = os.path.expanduser(raw_prefix)
    resolved_prefix = Path(expanded_prefix).resolve()

    if resolved_prefix == Path("/usr/local").resolve():
        raise DevError("refusing to install to /usr/local.")

    if resolved_prefix != Path(default_expanded).resolve():
        confirm = input(
            f"Install to non-default prefix '{expanded_prefix}'? Type 'yes' to confirm: "
        ).strip()
        if confirm != "yes":
            raise DevError("non-default install prefix was not confirmed with 'yes'.")

    inst_res = run_make(root, "install", f"PREFIX={expanded_prefix}")
    if inst_res.returncode != 0:
        raise DevError(f"make install PREFIX={expanded_prefix} failed:\n{inst_res.stdout.strip()}")

    type_res = subprocess.run(
        ["bash", "-c", "type -a tam"],
        capture_output=True,
        text=True,
        check=False,
    )
    type_out = type_res.stdout.strip()
    if type_out:
        print(type_out)
    expected_bin = str(Path(expanded_prefix) / "bin" / "tam")
    first_line = type_out.splitlines()[0] if type_out else ""
    first_path = first_line.split(" is ", 1)[1].strip() if " is " in first_line else ""
    if not first_path or Path(first_path).resolve() != Path(expected_bin).resolve():
        raise DevError(
            f"expected first entry of 'type -a tam' to be {expected_bin}, got {first_line or '(none)'}"
        )

    ver_res = subprocess.run(
        [expected_bin, "--version"],
        capture_output=True,
        text=True,
        check=False,
    )
    if ver_res.returncode != 0:
        raise DevError(f"{expected_bin} --version failed:\n{ver_res.stderr.strip()}")
    print(ver_res.stdout.rstrip())
    print("Reminder: run 'hash -r' in your interactive shell to refresh command lookup.")

    return raw_prefix


# ---------------------------------------------------------------------------
# 9. Push
# ---------------------------------------------------------------------------

def step_push(root):
    print("== 9. Push ==")
    run_git(root, "fetch")
    ahead_res = run_git(root, "log", "--oneline", "HEAD..@{u}")
    upstream_commits = ahead_res.stdout.strip()
    if upstream_commits:
        print("Upstream has commits not in HEAD; not pushing:")
        print(upstream_commits)
        return "not pushed (upstream has new commits)"

    ans = input("Push develop? [y/N]: ").strip().lower()
    if ans in ("y", "yes"):
        run_git(root, "push", "origin", "develop")
        print("Pushed develop to origin.")
        return "pushed origin/develop"
    print("Skipped push.")
    return "not pushed"


# ---------------------------------------------------------------------------
# 10. Finish
# ---------------------------------------------------------------------------

def step_finish(root, claim_state, committed, version_info, prefix, push_state):
    print("== 10. Finish ==")
    release_claim(root, claim_state, note="make dev complete")
    commit_str = ", ".join(f"{sha} ({subj})" for sha, subj in committed) or "none"
    versions = []
    if version_info.get("corpus_version"):
        versions.append(f"corpus {version_info['corpus_version']}")
    if version_info.get("command_version"):
        versions.append(f"command {version_info['command_version']}")
    version_str = ", ".join(versions) if versions else "unchanged"
    print(f"Commits: {commit_str}")
    print(f"Versions: {version_str}")
    print(f"Install prefix: {prefix}")
    print(f"Push state: {push_state}")


def format_commit_status(committed):
    if not committed:
        return "Nothing was committed."
    done = ", ".join(f"{sha} ({subj})" for sha, subj in committed)
    return f"Committed before failure: {done}. Remaining steps were not committed."


def main(root=None, dry_run=False):
    root = Path(root).resolve() if root else Path(__file__).resolve().parent.parent
    ts = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    claim_state = {
        "owner": "fred",
        "session": f"make-dev-{ts}",
        "active": False,
    }
    committed = []

    def handle_signal(signum, _frame):
        release_claim(root, claim_state, note=f"make dev interrupted (signal {signum})")
        print(
            f"\nInterrupted by signal {signum}. {format_commit_status(committed)}",
            file=sys.stderr,
        )
        sys.exit(128 + signum)

    old_sigint = signal.signal(signal.SIGINT, handle_signal)
    old_sigterm = signal.signal(signal.SIGTERM, handle_signal)

    try:
        step_preflight(root, dry_run=dry_run)
        changes = scan_working_tree(root, claim_state["owner"], claim_state["session"])
        if not dry_run:
            step_claim(root, changes["paths"], claim_state)
        changes = step_collect_changes(root, claim_state, dry_run=dry_run, changes=changes)
        if changes is None:
            return 0
        version_info = step_versions(root, changes, claim_state, dry_run=dry_run)
        if dry_run:
            return 0
        step_build_and_test(root, changes)
        main_sha = step_commit(root, changes, version_info, committed)
        step_index_followup(root, main_sha, claim_state, committed)
        prefix = step_install(root)
        push_state = step_push(root)
        step_finish(root, claim_state, committed, version_info, prefix, push_state)
        return 0
    except DevError as err:
        print(f"Error: {err} {format_commit_status(committed)}", file=sys.stderr)
        return 1
    finally:
        release_claim(root, claim_state, note="make dev cleanup")
        signal.signal(signal.SIGINT, old_sigint)
        signal.signal(signal.SIGTERM, old_sigterm)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="Tam command checkout (used for isolated fixtures)")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Run steps 1, 3 and 4 as a preview without writing, building or committing",
    )
    parsed = parser.parse_args()
    sys.exit(main(root=parsed.root, dry_run=parsed.dry_run))
