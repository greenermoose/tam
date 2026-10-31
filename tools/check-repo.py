#!/usr/bin/env python3
"""check-repo.py: Deterministic drift checks across copied repository facts.

Usage: check-repo.py [--root DIR]

Checks the copied-fact invariants defined in docs/plans/dev-loop.md (TCK12, TCK13):
  1. Command version (src/version.h -> tests/run.sh, ROADMAP.md, development.md)
  2. Corpus version (knowledge/version -> ROADMAP.md)
  3. Build inputs (src/*.c and src/*.h -> Makefile SRC and DEPS)
  4. Plan index (docs/plans/**/*.md -> docs/plans/README.md)
  5. Repository map (top-level entries and docs/ subdirectories -> docs/repo-map.md)
  6. Relative links (tracked *.md files -> existing target paths)
  7. Lane table (AGENTS.md lane table -> existing paths)
  8. Pager fixtures (tests/run.sh interactive tests -> tests/fixtures/, not knowledge/)

Prints one finding per line naming the file, line, expected value and fix.
Exits 0 when clean, 1 when drift is found.
"""

import argparse
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys

CATEGORIES = (
    "you edit",
    "script edits",
    "generated, never edit",
    "history, read-only",
)


def format_finding(rel_file, line, check, problem, expected, fix):
    return (
        f"{rel_file}:{line}: {check}: {problem}; "
        f"expected {expected}; fix: {fix}"
    )


def read_lines(root, rel_path):
    path = root / rel_path
    if not path.is_file():
        return None
    return path.read_text(encoding="utf-8", errors="replace").splitlines()


def parse_roadmap_sections(lines):
    """Locate header versions, section 2 delivered-history rows, and section 3 develop row."""
    cmd_header = None
    corpus_header = None
    develop_row = None
    history_rows = []
    in_section_2 = False
    section_2_last_line = 1

    header_cmd_re = re.compile(r"\*\*Current Command Version:\*\*\s*`([^`]+)`")
    header_corpus_re = re.compile(r"\*\*Current Corpus Version:\*\*\s*`([^`]+)`")
    develop_re = re.compile(r"^\|\s*`develop`\s*\|\s*([^|/]+?)\s*/\s*([^|]+?)\s*\|")
    table_row_re = re.compile(r"^\|\s*\*\*([^*]+)\*\*\s*\|")

    for idx, line in enumerate(lines, 1):
        m_cmd = header_cmd_re.search(line)
        if m_cmd and cmd_header is None:
            cmd_header = (idx, m_cmd.group(1).strip())
        m_cor = header_corpus_re.search(line)
        if m_cor and corpus_header is None:
            corpus_header = (idx, m_cor.group(1).strip())

        if line.startswith("## 2. Delivered history") or line.startswith("## 3. Delivered history"):
            in_section_2 = True
            section_2_last_line = idx
            continue
        if in_section_2 and line.startswith("## "):
            in_section_2 = False

        if in_section_2:
            if line.strip().startswith("|"):
                section_2_last_line = idx
            m_row = table_row_re.match(line)
            if m_row:
                history_rows.append((idx, m_row.group(1).strip()))

        m_dev = develop_re.match(line)
        if m_dev and develop_row is None:
            develop_row = (idx, m_dev.group(1).strip(), m_dev.group(2).strip())

    return {
        "cmd_header": cmd_header,
        "corpus_header": corpus_header,
        "develop_row": develop_row,
        "history_rows": history_rows,
        "section_2_last_line": section_2_last_line,
    }


# ---------------------------------------------------------------------------
# 1. Command version
# ---------------------------------------------------------------------------

def check_command_version(root):
    findings = []
    ver_lines = read_lines(root, "src/version.h")
    if ver_lines is None:
        return [
            format_finding(
                "src/version.h", 1, "Command version",
                "missing src/version.h",
                "'#define TAM_COMMAND_VERSION \"X.Y.Z\"'",
                "restore src/version.h",
            )
        ]

    cmd_ver = None
    for idx, line in enumerate(ver_lines, 1):
        m = re.match(r'^#define\s+TAM_COMMAND_VERSION\s+"([^"]+)"', line)
        if m:
            cmd_ver = m.group(1).strip()
            break

    if not cmd_ver:
        return [
            format_finding(
                "src/version.h", 1, "Command version",
                "TAM_COMMAND_VERSION macro not found",
                "'#define TAM_COMMAND_VERSION \"X.Y.Z\"'",
                "define TAM_COMMAND_VERSION in src/version.h",
            )
        ]

    # Copy 1: tests/run.sh assertion
    run_lines = read_lines(root, "tests/run.sh")
    if run_lines is None:
        findings.append(
            format_finding(
                "tests/run.sh", 1, "Command version",
                "missing tests/run.sh",
                f"'^tam command {cmd_ver}' assertion (from src/version.h)",
                "restore tests/run.sh",
            )
        )
    else:
        found_assert = False
        for idx, line in enumerate(run_lines, 1):
            m = re.search(r"\^tam command ([0-9]+\.[0-9]+\.[0-9]+)", line)
            if m:
                found_assert = True
                actual = m.group(1)
                if actual != cmd_ver:
                    findings.append(
                        format_finding(
                            "tests/run.sh", idx, "Command version",
                            f"--version assertion has '{actual}'",
                            f"'^tam command {cmd_ver}' (from src/version.h)",
                            f"update tests/run.sh line {idx} to '^tam command {cmd_ver}'",
                        )
                    )
        if not found_assert:
            findings.append(
                format_finding(
                    "tests/run.sh", 1, "Command version",
                    "missing '--version' command assertion",
                    f"'^tam command {cmd_ver}' (from src/version.h)",
                    f"add grep -q '^tam command {cmd_ver}' assertion in tests/run.sh",
                )
            )

    # Copies 2, 3, 4: ROADMAP.md header, develop row, and delivered-history row
    rm_lines = read_lines(root, "ROADMAP.md")
    if rm_lines is None:
        findings.append(
            format_finding(
                "ROADMAP.md", 1, "Command version",
                "missing ROADMAP.md",
                f"command version '{cmd_ver}' (from src/version.h)",
                "restore ROADMAP.md",
            )
        )
    else:
        rm = parse_roadmap_sections(rm_lines)
        if rm["cmd_header"] is None:
            findings.append(
                format_finding(
                    "ROADMAP.md", 1, "Command version",
                    "missing Current Command Version header",
                    f"'**Current Command Version:** `{cmd_ver}`.' (from src/version.h)",
                    f"set '**Current Command Version:** `{cmd_ver}`.' in ROADMAP.md",
                )
            )
        elif rm["cmd_header"][1] != cmd_ver:
            line_no, actual = rm["cmd_header"]
            findings.append(
                format_finding(
                    "ROADMAP.md", line_no, "Command version",
                    f"Current Command Version header has '{actual}'",
                    f"'{cmd_ver}' (from src/version.h)",
                    f"update ROADMAP.md line {line_no} to '**Current Command Version:** `{cmd_ver}`.'",
                )
            )

        if rm["develop_row"] is None:
            findings.append(
                format_finding(
                    "ROADMAP.md", 1, "Command version",
                    "missing `develop` row in section 3",
                    f"'| `develop` | {cmd_ver} / <corpus> |' (from src/version.h)",
                    f"add the `develop` row with command version {cmd_ver} in section 3 of ROADMAP.md",
                )
            )
        elif rm["develop_row"][1] != cmd_ver:
            line_no, actual_cmd, actual_cor = rm["develop_row"]
            findings.append(
                format_finding(
                    "ROADMAP.md", line_no, "Command version",
                    f"`develop` row has command version '{actual_cmd}'",
                    f"'{cmd_ver} / {actual_cor}' (from src/version.h)",
                    f"update ROADMAP.md line {line_no} to '| `develop` | {cmd_ver} / {actual_cor} |'",
                )
            )

        cmd_hist_re = re.compile(
            r"(?:^|\b)" + re.escape(cmd_ver) + r"(?:\s+command\b|$)"
        )
        has_cmd_history = any(
            cmd_hist_re.search(cell) and not re.search(re.escape(cmd_ver) + r"\s+corpus\b", cell)
            for _, cell in rm["history_rows"]
        )
        if not has_cmd_history:
            findings.append(
                format_finding(
                    "ROADMAP.md", rm["section_2_last_line"], "Command version",
                    f"delivered-history table has no row naming command version '{cmd_ver}'",
                    f"a delivered-history row naming '**{cmd_ver} command**' (from src/version.h)",
                    f"add a '| **{cmd_ver} command** | ... |' row to section 2 of ROADMAP.md",
                )
            )

    # Copy 5: docs/plans/development.md decision-record entry
    dev_lines = read_lines(root, "docs/plans/development.md")
    if dev_lines is None:
        findings.append(
            format_finding(
                "docs/plans/development.md", 1, "Command version",
                "missing docs/plans/development.md",
                f"decision-record entry naming 'Command {cmd_ver}' (from src/version.h)",
                "restore docs/plans/development.md",
            )
        )
    else:
        in_decisions = False
        decision_line = 1
        found_decision = False
        dec_re = re.compile(r"\b(?:Command|tam)\s+" + re.escape(cmd_ver) + r"\b")
        for idx, line in enumerate(dev_lines, 1):
            if line.startswith("## Decision record"):
                in_decisions = True
                decision_line = idx
                continue
            if in_decisions and line.startswith("## "):
                in_decisions = False
            if in_decisions and dec_re.search(line):
                found_decision = True
                break
        if not found_decision:
            findings.append(
                format_finding(
                    "docs/plans/development.md", decision_line, "Command version",
                    f"decision record has no entry naming command version '{cmd_ver}'",
                    f"'- **YYYY-MM-DD:** Command {cmd_ver} (patch|minor): <summary>' (from src/version.h)",
                    f"add a decision-record entry for Command {cmd_ver} under '## Decision record' in docs/plans/development.md",
                )
            )

    return findings


# ---------------------------------------------------------------------------
# 2. Corpus version
# ---------------------------------------------------------------------------

def check_corpus_version(root):
    findings = []
    ver_lines = read_lines(root, "knowledge/version")
    if not ver_lines or not ver_lines[0].strip():
        return [
            format_finding(
                "knowledge/version", 1, "Corpus version",
                "missing or empty knowledge/version",
                "corpus version 'X.Y.Z'",
                "write the current corpus version to knowledge/version",
            )
        ]
    corpus_ver = ver_lines[0].strip()

    rm_lines = read_lines(root, "ROADMAP.md")
    if rm_lines is None:
        return [
            format_finding(
                "ROADMAP.md", 1, "Corpus version",
                "missing ROADMAP.md",
                f"corpus version '{corpus_ver}' (from knowledge/version)",
                "restore ROADMAP.md",
            )
        ]

    rm = parse_roadmap_sections(rm_lines)
    if rm["corpus_header"] is None:
        findings.append(
            format_finding(
                "ROADMAP.md", 1, "Corpus version",
                "missing Current Corpus Version header",
                f"'**Current Corpus Version:** `{corpus_ver}`.' (from knowledge/version)",
                f"set '**Current Corpus Version:** `{corpus_ver}`.' in ROADMAP.md",
            )
        )
    elif rm["corpus_header"][1] != corpus_ver:
        line_no, actual = rm["corpus_header"]
        findings.append(
            format_finding(
                "ROADMAP.md", line_no, "Corpus version",
                f"Current Corpus Version header has '{actual}'",
                f"'{corpus_ver}' (from knowledge/version)",
                f"update ROADMAP.md line {line_no} to '**Current Corpus Version:** `{corpus_ver}`.'",
            )
        )

    if rm["develop_row"] is None:
        findings.append(
            format_finding(
                "ROADMAP.md", 1, "Corpus version",
                "missing `develop` row in section 3",
                f"'| `develop` | <command> / {corpus_ver} |' (from knowledge/version)",
                f"add the `develop` row with corpus version {corpus_ver} in section 3 of ROADMAP.md",
            )
        )
    elif rm["develop_row"][2] != corpus_ver:
        line_no, actual_cmd, actual_cor = rm["develop_row"]
        findings.append(
            format_finding(
                "ROADMAP.md", line_no, "Corpus version",
                f"`develop` row has corpus version '{actual_cor}'",
                f"'{actual_cmd} / {corpus_ver}' (from knowledge/version)",
                f"update ROADMAP.md line {line_no} to '| `develop` | {actual_cmd} / {corpus_ver} |'",
            )
        )

    corpus_hist_re = re.compile(r"\b" + re.escape(corpus_ver) + r"\s+corpus\b")
    has_corpus_history = any(
        corpus_hist_re.search(cell) or cell == corpus_ver
        for _, cell in rm["history_rows"]
    )
    if not has_corpus_history:
        findings.append(
            format_finding(
                "ROADMAP.md", rm["section_2_last_line"], "Corpus version",
                f"delivered-history table has no row naming corpus version '{corpus_ver}'",
                f"a delivered-history row naming '**{corpus_ver} corpus**' (from knowledge/version)",
                f"add a '| **{corpus_ver} corpus** | ... |' row to section 2 of ROADMAP.md",
            )
        )

    return findings


# ---------------------------------------------------------------------------
# 3. Build inputs
# ---------------------------------------------------------------------------

def parse_makefile_var(lines, var_name):
    """Return (line_no, list_of_tokens) for `var_name := ...` (supporting backslash continuations)."""
    pattern = re.compile(r"^" + re.escape(var_name) + r"\s*:?=\s*(.*)$")
    for idx, line in enumerate(lines, 1):
        m = pattern.match(line)
        if m:
            chunks = [m.group(1)]
            cur = idx - 1
            while chunks[-1].rstrip().endswith("\\") and cur + 1 < len(lines):
                chunks[-1] = chunks[-1].rstrip()[:-1]
                cur += 1
                chunks.append(lines[cur])
            tokens = " ".join(chunks).split()
            return idx, tokens
    return None, []


def check_build_inputs(root):
    findings = []
    mk_lines = read_lines(root, "Makefile")
    if mk_lines is None:
        return [
            format_finding(
                "Makefile", 1, "Build inputs",
                "missing Makefile",
                "SRC and DEPS matching src/*.c and src/*.h",
                "restore Makefile",
            )
        ]

    src_dir = root / "src"
    disk_c = sorted(f"src/{p.name}" for p in src_dir.glob("*.c")) if src_dir.is_dir() else []
    disk_h = sorted(f"src/{p.name}" for p in src_dir.glob("*.h")) if src_dir.is_dir() else []

    for var_name, disk_files, glob_desc in (
        ("SRC", disk_c, "src/*.c"),
        ("DEPS", disk_h, "src/*.h"),
    ):
        line_no, listed = parse_makefile_var(mk_lines, var_name)
        if line_no is None:
            findings.append(
                format_finding(
                    "Makefile", 1, "Build inputs",
                    f"{var_name} definition not found",
                    f"{var_name} listing all {glob_desc} files ({' '.join(disk_files)})",
                    f"define {var_name} in Makefile",
                )
            )
            continue

        listed_set = set(listed)
        disk_set = set(disk_files)
        for missing in sorted(disk_set - listed_set):
            findings.append(
                format_finding(
                    "Makefile", line_no, "Build inputs",
                    f"{var_name} is missing '{missing}' (present on disk in src/)",
                    f"'{missing}' in {var_name}",
                    f"add '{missing}' to {var_name} on Makefile line {line_no}",
                )
            )
        for extra in sorted(listed_set - disk_set):
            findings.append(
                format_finding(
                    "Makefile", line_no, "Build inputs",
                    f"{var_name} lists missing file '{extra}' (not found in src/)",
                    f"only existing {glob_desc} files in {var_name}",
                    f"remove '{extra}' from {var_name} on Makefile line {line_no} or create {extra}",
                )
            )

    return findings


# ---------------------------------------------------------------------------
# 4. Plan index
# ---------------------------------------------------------------------------

def check_plan_index(root):
    findings = []
    plans_dir = root / "docs" / "plans"
    readme_rel = "docs/plans/README.md"
    readme_lines = read_lines(root, readme_rel)
    if readme_lines is None:
        return [
            format_finding(
                readme_rel, 1, "Plan index",
                "missing docs/plans/README.md",
                "one index row per Markdown file under docs/plans/",
                "restore docs/plans/README.md",
            )
        ]

    disk_plans = set()
    if plans_dir.is_dir():
        for p in plans_dir.rglob("*.md"):
            rel = p.relative_to(plans_dir).as_posix()
            if rel != "README.md":
                disk_plans.add(rel)

    row_link_re = re.compile(r"^\|\s*\[(?:`[^`]*`|[^\[\]])+\]\(([^)]+)\)\s*\|")
    rows_by_target = {}
    for idx, line in enumerate(readme_lines, 1):
        m = row_link_re.match(line)
        if not m:
            continue
        raw_target = m.group(1).strip()
        target = raw_target.split("#", 1)[0].strip()
        if not target or "://" in target:
            continue
        norm = PurePosixPath(target).as_posix()
        rows_by_target.setdefault(norm, []).append(idx)

    for target, line_nums in sorted(rows_by_target.items()):
        if target not in disk_plans:
            findings.append(
                format_finding(
                    readme_rel, line_nums[0], "Plan index",
                    f"index row links to missing file 'docs/plans/{target}'",
                    "only existing Markdown files under docs/plans/",
                    f"remove the row for '{target}' on {readme_rel}:{line_nums[0]} or create docs/plans/{target}",
                )
            )
        if len(line_nums) > 1:
            for dup_line in line_nums[1:]:
                findings.append(
                    format_finding(
                        readme_rel, dup_line, "Plan index",
                        f"duplicate index row for 'docs/plans/{target}' (first on line {line_nums[0]})",
                        f"exactly one row for '{target}' in {readme_rel}",
                        f"remove the duplicate row on {readme_rel}:{dup_line}",
                    )
                )

    last_line = len(readme_lines) or 1
    for plan_rel in sorted(disk_plans):
        if plan_rel not in rows_by_target:
            findings.append(
                format_finding(
                    readme_rel, last_line, "Plan index",
                    f"missing index row for 'docs/plans/{plan_rel}'",
                    f"exactly one table row linking to '{plan_rel}' in {readme_rel}",
                    f"add a row for '{plan_rel}' to {readme_rel}",
                )
            )

    return findings


# ---------------------------------------------------------------------------
# 5. Repository map
# ---------------------------------------------------------------------------

def expected_repo_map_entries(root):
    """Return dict mapping normalized entry key -> display name for top-level and docs/ subdirs."""
    entries = {}
    for p in sorted(root.iterdir(), key=lambda x: x.name):
        if p.name.startswith(".") or p.name == "__pycache__":
            continue
        entries[p.name] = f"{p.name}/" if p.is_dir() else p.name

    # build/ is generated by `make command` and may be absent right after `make clean`
    if "build" not in entries:
        gitignore = root / ".gitignore"
        makefile = root / "Makefile"
        has_build = (
            gitignore.is_file() and "build" in gitignore.read_text(encoding="utf-8", errors="replace")
        ) or (
            makefile.is_file() and re.search(r"^BUILD\s*:?=\s*build\b", makefile.read_text(encoding="utf-8", errors="replace"), re.M)
        )
        if has_build:
            entries["build"] = "build/"

    docs_dir = root / "docs"
    if docs_dir.is_dir():
        for p in sorted(docs_dir.iterdir(), key=lambda x: x.name):
            if p.is_dir() and not p.name.startswith(".") and p.name != "__pycache__":
                key = f"docs/{p.name}"
                entries[key] = f"docs/{p.name}/"

    return entries


def check_repo_map(root):
    findings = []
    map_rel = "docs/repo-map.md"
    map_lines = read_lines(root, map_rel)
    if map_lines is None:
        return [
            format_finding(
                map_rel, 1, "Repository map",
                "missing docs/repo-map.md",
                "one line per top-level entry and docs/ subdirectory",
                "create docs/repo-map.md",
            )
        ]

    expected = expected_repo_map_entries(root)
    seen = {}
    entry_bullet_re = re.compile(r"^\s*[-*]\s+`([^`]+)`")

    for idx, line in enumerate(map_lines, 1):
        code_matches = re.findall(r"`([^`]+)`", line)
        lower = line.lower()
        found_cat = None
        for cat in CATEGORIES:
            if cat in lower:
                found_cat = cat
                break

        m_bullet = entry_bullet_re.match(line)
        if not m_bullet and not (code_matches and found_cat):
            continue

        raw_entry = (m_bullet.group(1) if m_bullet else code_matches[0]).strip()
        key = raw_entry.rstrip("/")
        seen.setdefault(key, []).append((idx, raw_entry, found_cat))

    for key, occurrences in sorted(seen.items()):
        first_line, raw_entry, found_cat = occurrences[0]
        if found_cat is None:
            findings.append(
                format_finding(
                    map_rel, first_line, "Repository map",
                    f"entry '{raw_entry}' has no valid category",
                    f"one of {', '.join(repr(c) for c in CATEGORIES)}",
                    f"mark '{raw_entry}' with a valid category on {map_rel}:{first_line}",
                )
            )
        if key not in expected:
            findings.append(
                format_finding(
                    map_rel, first_line, "Repository map",
                    f"line for missing entry '{raw_entry}'",
                    "only existing top-level entries and docs/ subdirectories",
                    f"remove '{raw_entry}' on {map_rel}:{first_line} or create {raw_entry}",
                )
            )
        if len(occurrences) > 1:
            for dup_line, dup_raw, _ in occurrences[1:]:
                findings.append(
                    format_finding(
                        map_rel, dup_line, "Repository map",
                        f"duplicate line for '{dup_raw}' (first on line {first_line})",
                        f"exactly one line for '{dup_raw}' in {map_rel}",
                        f"remove the duplicate '{dup_raw}' line on {map_rel}:{dup_line}",
                    )
                )

    last_line = len(map_lines) or 1
    for key, display in sorted(expected.items()):
        if key not in seen:
            findings.append(
                format_finding(
                    map_rel, last_line, "Repository map",
                    f"missing line for '{display}'",
                    f"one line describing '{display}' in {map_rel}",
                    f"add a '- `{display}` — *<category>*. ...' line to {map_rel}",
                )
            )

    return findings


# ---------------------------------------------------------------------------
# 6. Relative links
# ---------------------------------------------------------------------------

def list_tracked_markdown(root):
    res = subprocess.run(
        ["git", "-C", str(root), "ls-files", "*.md"],
        capture_output=True,
        text=True,
        check=False,
    )
    if res.returncode == 0 and res.stdout.strip():
        return sorted(res.stdout.splitlines())
    return sorted(
        p.relative_to(root).as_posix()
        for p in root.rglob("*.md")
        if ".git" not in p.parts
    )


def check_relative_links(root):
    findings = []
    token_re = re.compile(r"(!?\[(?:`[^`]*`|[^\[\]])*\]\(([^)]+)\))|`[^`]+`")
    scheme_re = re.compile(r"^[a-zA-Z][a-zA-Z0-9+.-]*:")

    for md_rel in list_tracked_markdown(root):
        lines = read_lines(root, md_rel)
        if lines is None:
            continue
        md_parent = (root / md_rel).parent
        in_fence = False
        for idx, line in enumerate(lines, 1):
            if re.match(r"^\s*(`{3,}|~{3,})", line):
                in_fence = not in_fence
                continue
            if in_fence:
                continue
            for m in token_re.finditer(line):
                raw_target = m.group(2)
                if raw_target is None:
                    continue
                target = raw_target.strip().strip("<>")
                if not target:
                    continue
                target = target.split()[0]
                if target.startswith("#") or scheme_re.match(target) or target.startswith("/"):
                    continue
                path_part = target.split("#", 1)[0].strip()
                if not path_part:
                    continue
                resolved = md_parent / path_part
                if not resolved.exists():
                    try:
                        expected_rel = resolved.resolve().relative_to(root.resolve()).as_posix()
                    except ValueError:
                        expected_rel = path_part
                    findings.append(
                        format_finding(
                            md_rel, idx, "Relative links",
                            f"broken relative link '{raw_target.strip()}'",
                            f"target '{expected_rel}' to exist",
                            f"update link '{raw_target.strip()}' on {md_rel}:{idx} or create '{expected_rel}'",
                        )
                    )

    return findings


# ---------------------------------------------------------------------------
# 7. Lane table
# ---------------------------------------------------------------------------

def check_lane_table(root):
    findings = []
    agents_rel = "AGENTS.md"
    lines = read_lines(root, agents_rel)
    if lines is None:
        return [
            format_finding(
                agents_rel, 1, "Lane table",
                "missing AGENTS.md",
                "'## Lanes' table with existing repository paths",
                "restore AGENTS.md",
            )
        ]

    in_lanes = False
    lanes_line = 1
    checked_any = False
    for idx, line in enumerate(lines, 1):
        if line.startswith("## Lanes"):
            in_lanes = True
            lanes_line = idx
            continue
        if in_lanes and line.startswith("## "):
            break
        if not in_lanes or not line.strip().startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 2:
            continue
        lane_name = cells[0]
        if lane_name in ("Lane", "") or set(lane_name) <= {"-", ":", " "}:
            continue
        for raw_path in re.findall(r"`([^`]+)`", cells[1]):
            checked_any = True
            clean = raw_path.rstrip("/")
            target = root / clean
            if not target.exists():
                findings.append(
                    format_finding(
                        agents_rel, idx, "Lane table",
                        f"lane '{lane_name}' lists missing path '{raw_path}'",
                        f"path '{raw_path}' to exist in repository",
                        f"update the {lane_name} lane row on {agents_rel}:{idx} or create '{raw_path}'",
                    )
                )

    if not checked_any:
        findings.append(
            format_finding(
                agents_rel, lanes_line, "Lane table",
                "no lane paths found under '## Lanes'",
                "a Markdown table listing lane paths in backticks",
                "restore the '## Lanes' table in AGENTS.md",
            )
        )

    return findings


# ---------------------------------------------------------------------------
# 8. Pager tests on fixtures
# ---------------------------------------------------------------------------

LIVE_CORPUS_RE = re.compile(r'"\$root/knowledge"|"\$knowledge"')
PTY_LAUNCH_RE = re.compile(r"""^'\s|python3\s+"\$here/""")


def check_pager_fixtures(root):
    """Interactive tests in tests/run.sh must read tests/fixtures/, not the
    live corpus, so a page edit cannot break a command test. A deliberate
    exception carries '# live-corpus: <reason>' on the line before."""
    findings = []
    run_rel = "tests/run.sh"
    lines = read_lines(root, run_rel)
    if lines is None:
        return findings
    for idx, line in enumerate(lines, 1):
        if not PTY_LAUNCH_RE.search(line) or not LIVE_CORPUS_RE.search(line):
            continue
        if idx > 1 and "# live-corpus:" in lines[idx - 2]:
            continue
        findings.append(
            format_finding(
                run_rel, idx, "Pager fixtures",
                "an interactive test reads the live corpus",
                "a root under tests/fixtures/ (for example tests/fixtures/guide)",
                f"point the test on {run_rel}:{idx} at a fixture, or put "
                "'# live-corpus: <reason>' on the line before it",
            )
        )
    return findings


def check_repo(root):
    root = Path(root)
    findings = []
    findings.extend(check_command_version(root))
    findings.extend(check_corpus_version(root))
    findings.extend(check_build_inputs(root))
    findings.extend(check_plan_index(root))
    findings.extend(check_repo_map(root))
    findings.extend(check_relative_links(root))
    findings.extend(check_lane_table(root))
    findings.extend(check_pager_fixtures(root))
    return findings


def main(argv=None):
    parser = argparse.ArgumentParser(description="Check copied repository facts for drift.")
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="Repository root directory",
    )
    args = parser.parse_args(argv)
    findings = check_repo(args.root)
    for item in findings:
        print(item)
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
