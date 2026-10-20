"""Behavioral and fixture tests for tools/check-repo.py (TCK12)."""

import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

CHECK_SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "check-repo.py"
DEV_SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "dev.py"

SPEC = importlib.util.spec_from_file_location("tam_check_repo", CHECK_SCRIPT)
check_mod = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(check_mod)


ROADMAP_FIXTURE = """# tam Roadmap

**Reviewed:** 2026-10-20. **Owner:** Fred (developer).
**Current Command Version:** `0.32.0`. **Current Corpus Version:** `0.34.0`.

Read the [plan index](docs/plans/README.md) and [development contract](docs/plans/development.md).

## 2. Delivered history

| Version | Milestone / Focus | Status | Key Delivered Capabilities |
| :--- | :--- | :--- | :--- |
| **0.32.0 command / 0.33.5 corpus** | Home-row pager keys | Done (2026-10-20) | Home-row navigation. |
| **0.34.0 corpus** | POSIX manual page | Done (2026-10-20) | POSIX Programmer's Manual guide. |

## 3. Current state and follow-up

| Branch | Command / corpus | Meaning |
| --- | --- | --- |
| `develop` | 0.32.0 / 0.34.0 | Newest source. |
"""

DEVELOPMENT_FIXTURE = """# tam development plan

**Plan status:** Active
**Reviewed:** 2026-10-20
**Scope:** Living integration contract.
**Authority:** [roadmap](../../ROADMAP.md) and [plan index](README.md).
**Next action:** Maintain contract.

## Decision record

- **2026-10-20:** Command 0.32.0 (minor): Home-row pager keys.
"""

DEV_LOOP_FIXTURE = """# Developer loop

**Plan status:** In progress
**Reviewed:** 2026-10-20
**Scope:** Tooling.
**Authority:** [development contract](development.md#decision-record).
**Next action:** Developer acceptance.
"""

PLANS_README_FIXTURE = """# tam plan index

| Plan | Status | Disposition |
| --- | --- | --- |
| [Developer loop](dev-loop.md) | In progress | TCK11 and TCK12 implemented. |
| [Development contract](development.md) | Active | Living contract. |
"""

AGENTS_FIXTURE = """# Working on tam with an AI agent

See [ROADMAP.md](ROADMAP.md).

## Lanes

| Lane | Paths | Build | Tests |
| --- | --- | --- | --- |
| Command | `src/`, `tests/run.sh` | `make command` | `make test-command` |
| Corpus | `knowledge/`, `tools/` | `make corpus` | `make corpus-and-test` |
| Shared | `Makefile`, `ROADMAP.md`, `README.md`, `docs/plans/` | | `make test` |
"""

MAKEFILE_FIXTURE = """BUILD := build
SRC   := src/tam.c src/render.c
DEPS  := src/version.h src/render.h
"""

REPO_MAP_FIXTURE = """# Repository map

Categories:

- *you edit* — authored source and docs
- *script edits* — version places updated by the developer loop
- *generated, never edit* — build outputs
- *history, read-only* — historical records

## Entries

- `AGENTS.md` — *you edit*. Agent rules and lane table.
- `Makefile` — *you edit*. Build and test targets.
- `README.md` — *you edit*. Overview and command reference.
- `ROADMAP.md` — *script edits*. Component versions and delivered history.
- `build/` — *generated, never edit*. Compiled binary output.
- `docs/` — *you edit*. Project documentation.
- `docs/ai/` — *history, read-only*. Per-session provenance records.
- `docs/plans/` — *you edit*. Feature plans and plan index.
- `knowledge/` — *you edit*. HTML corpus pages and version file.
- `src/` — *you edit*. C source and headers.
- `tests/` — *you edit*. Automated test suites.
- `tools/` — *you edit*. Build and verification scripts.
"""


class CheckRepoTest(unittest.TestCase):
    def make_clean_fixture(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        root = Path(temp.name)

        (root / "src").mkdir()
        (root / "knowledge").mkdir()
        (root / "tests").mkdir()
        (root / "tools").mkdir()
        (root / "docs" / "plans").mkdir(parents=True)
        (root / "docs" / "ai").mkdir(parents=True)

        (root / ".gitignore").write_text("build/\n__pycache__/\n", encoding="utf-8")
        (root / "Makefile").write_text(MAKEFILE_FIXTURE, encoding="utf-8")
        (root / "README.md").write_text(
            "# tam\n\nSee [ROADMAP](ROADMAP.md#1-vision) and `[ignored](missing.md)`.\n"
            "```markdown\n[fenced](also-missing.md)\n```\n",
            encoding="utf-8",
        )
        (root / "ROADMAP.md").write_text(ROADMAP_FIXTURE, encoding="utf-8")
        (root / "AGENTS.md").write_text(AGENTS_FIXTURE, encoding="utf-8")
        (root / "src" / "version.h").write_text(
            '#ifndef TAM_VERSION_H\n#define TAM_COMMAND_VERSION "0.32.0"\n#endif\n',
            encoding="utf-8",
        )
        (root / "src" / "render.h").write_text("#ifndef TAM_RENDER_H\n#endif\n", encoding="utf-8")
        (root / "src" / "tam.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
        (root / "src" / "render.c").write_text("int render(void) { return 0; }\n", encoding="utf-8")
        (root / "knowledge" / "version").write_text("0.34.0\n", encoding="utf-8")
        (root / "tests" / "run.sh").write_text(
            '#!/bin/sh\n"$TAM" --version | grep -q \'^tam command 0.32.0\' || exit 1\n',
            encoding="utf-8",
        )
        (root / "tools" / "check-repo.py").write_text("#!/usr/bin/env python3\n", encoding="utf-8")
        (root / "docs" / "repo-map.md").write_text(REPO_MAP_FIXTURE, encoding="utf-8")
        (root / "docs" / "plans" / "README.md").write_text(PLANS_README_FIXTURE, encoding="utf-8")
        (root / "docs" / "plans" / "development.md").write_text(
            DEVELOPMENT_FIXTURE, encoding="utf-8"
        )
        (root / "docs" / "plans" / "dev-loop.md").write_text(DEV_LOOP_FIXTURE, encoding="utf-8")
        (root / "docs" / "ai" / "README.md").write_text(
            "# AI records\n\nSee [ plans ](../plans/README.md).\n",
            encoding="utf-8",
        )

        subprocess.run(["git", "init", "-q", "-b", "develop"], cwd=root, check=True)
        subprocess.run(["git", "add", "."], cwd=root, check=True)
        return root

    def assert_finding_details(self, finding, expected_file, expected_line, expected_check):
        self.assertTrue(
            finding.startswith(f"{expected_file}:{expected_line}: {expected_check}:"),
            f"Unexpected finding prefix: {finding}",
        )
        self.assertIn("expected ", finding)
        self.assertIn("; fix: ", finding)

    def test_clean_fixture_passes(self):
        root = self.make_clean_fixture()
        findings = check_mod.check_repo(root)
        self.assertEqual(findings, [])
        res = subprocess.run(
            [sys.executable, str(CHECK_SCRIPT), "--root", str(root)],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertEqual(res.stdout.strip(), "")

    def test_command_version_drift_fails(self):
        # 1a. tests/run.sh assertion drift
        root = self.make_clean_fixture()
        (root / "tests" / "run.sh").write_text(
            '#!/bin/sh\n"$TAM" --version | grep -q \'^tam command 0.31.9\' || exit 1\n',
            encoding="utf-8",
        )
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "tests/run.sh", 2, "Command version")

        # 1b. ROADMAP.md header, develop row, and delivered-history row drift
        root = self.make_clean_fixture()
        (root / "src" / "version.h").write_text(
            '#define TAM_COMMAND_VERSION "0.33.0"\n', encoding="utf-8"
        )
        (root / "tests" / "run.sh").write_text(
            '#!/bin/sh\n"$TAM" --version | grep -q \'^tam command 0.33.0\' || exit 1\n',
            encoding="utf-8",
        )
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 4, findings)
        self.assertTrue(any("ROADMAP.md:4: Command version:" in f for f in findings))
        self.assertTrue(any("ROADMAP.md:19: Command version:" in f for f in findings))
        self.assertTrue(any("ROADMAP.md:13: Command version:" in f for f in findings))
        self.assertTrue(
            any("docs/plans/development.md:9: Command version:" in f for f in findings)
        )
        for f in findings:
            self.assertIn("expected ", f)
            self.assertIn("; fix: ", f)

    def test_corpus_version_drift_fails(self):
        root = self.make_clean_fixture()
        # Plant drift in ROADMAP.md develop row only
        (root / "ROADMAP.md").write_text(
            ROADMAP_FIXTURE.replace("| `develop` | 0.32.0 / 0.34.0 |", "| `develop` | 0.32.0 / 0.33.6 |"),
            encoding="utf-8",
        )
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "ROADMAP.md", 19, "Corpus version")

        # Plant drift in delivered-history table
        root2 = self.make_clean_fixture()
        (root2 / "ROADMAP.md").write_text(
            ROADMAP_FIXTURE.replace(
                "| **0.34.0 corpus** | POSIX manual page | Done (2026-10-20) | POSIX Programmer's Manual guide. |\n",
                "",
            ),
            encoding="utf-8",
        )
        findings2 = check_mod.check_repo(root2)
        self.assertEqual(len(findings2), 1, findings2)
        self.assert_finding_details(findings2[0], "ROADMAP.md", 12, "Corpus version")

    def test_build_inputs_drift_fails(self):
        root = self.make_clean_fixture()
        (root / "src" / "extra.c").write_text("int extra(void) { return 0; }\n", encoding="utf-8")
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "Makefile", 2, "Build inputs")
        self.assertIn("src/extra.c", findings[0])

        root2 = self.make_clean_fixture()
        (root2 / "src" / "render.h").unlink()
        findings2 = check_mod.check_repo(root2)
        self.assertEqual(len(findings2), 1, findings2)
        self.assert_finding_details(findings2[0], "Makefile", 3, "Build inputs")
        self.assertIn("src/render.h", findings2[0])

    def test_plan_index_drift_fails(self):
        # Unindexed plan file on disk
        root = self.make_clean_fixture()
        (root / "docs" / "plans" / "orphan.md").write_text("# Orphan\n", encoding="utf-8")
        subprocess.run(["git", "add", "docs/plans/orphan.md"], cwd=root, check=True)
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "docs/plans/README.md", 6, "Plan index")
        self.assertIn("orphan.md", findings[0])

        # Duplicate row in README.md for an existing plan file
        root2 = self.make_clean_fixture()
        (root2 / "docs" / "plans" / "README.md").write_text(
            PLANS_README_FIXTURE + "| [Developer loop](dev-loop.md) | In progress | Duplicate. |\n",
            encoding="utf-8",
        )
        findings2 = check_mod.check_repo(root2)
        self.assertEqual(len(findings2), 1, findings2)
        self.assert_finding_details(findings2[0], "docs/plans/README.md", 7, "Plan index")
        self.assertIn("dev-loop.md", findings2[0])

        # Row in README.md for missing plan file
        root3 = self.make_clean_fixture()
        (root3 / "docs" / "plans" / "README.md").write_text(
            PLANS_README_FIXTURE + "| [Ghost plan](ghost.md) | Ready | Missing file. |\n",
            encoding="utf-8",
        )
        findings3 = check_mod.check_plan_index(root3)
        self.assertEqual(len(findings3), 1, findings3)
        self.assert_finding_details(findings3[0], "docs/plans/README.md", 7, "Plan index")
        self.assertIn("ghost.md", findings3[0])

    def test_repository_map_drift_fails(self):
        # Missing entry in docs/repo-map.md for a new top-level directory
        root = self.make_clean_fixture()
        (root / "completions").mkdir()
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "docs/repo-map.md", 23, "Repository map")
        self.assertIn("completions/", findings[0])

        # Extra line in docs/repo-map.md for a missing entry
        root2 = self.make_clean_fixture()
        (root2 / "docs" / "repo-map.md").write_text(
            REPO_MAP_FIXTURE + "- `obsolete/` — *you edit*. Removed folder.\n",
            encoding="utf-8",
        )
        findings2 = check_mod.check_repo(root2)
        self.assertEqual(len(findings2), 1, findings2)
        self.assert_finding_details(findings2[0], "docs/repo-map.md", 24, "Repository map")
        self.assertIn("obsolete/", findings2[0])

    def test_dev_script_reads_repo_map_categories_and_warns_on_history(self):
        spec = importlib.util.spec_from_file_location("tam_dev", DEV_SCRIPT)
        dev_mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(dev_mod)

        root = self.make_clean_fixture()
        repo_map = dev_mod.load_repo_map(root)
        self.assertEqual(dev_mod.categorize_path("knowledge/welcome.html", repo_map), "you edit")
        self.assertEqual(dev_mod.categorize_path("ROADMAP.md", repo_map), "script edits")
        self.assertEqual(dev_mod.categorize_path("build/tam", repo_map), "generated, never edit")
        self.assertEqual(
            dev_mod.categorize_path("docs/ai/2026-10-20-rec.md", repo_map),
            "history, read-only",
        )
        self.assertEqual(dev_mod.categorize_path("docs/plans/dev-loop.md", repo_map), "you edit")

    def test_relative_links_drift_fails(self):
        root = self.make_clean_fixture()
        (root / "README.md").write_text(
            "# tam\n\nBroken link to [nonexistent guide](docs/no-such-file.md#sec).\n",
            encoding="utf-8",
        )
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "README.md", 3, "Relative links")
        self.assertIn("docs/no-such-file.md", findings[0])

    def test_lane_table_drift_fails(self):
        root = self.make_clean_fixture()
        (root / "AGENTS.md").write_text(
            AGENTS_FIXTURE.replace("`tests/run.sh`", "`tests/missing.sh`"),
            encoding="utf-8",
        )
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "AGENTS.md", 9, "Lane table")
        self.assertIn("tests/missing.sh", findings[0])

    def test_pager_test_on_live_corpus_fails(self):
        root = self.make_clean_fixture()
        run_sh = (root / "tests" / "run.sh").read_text(encoding="utf-8")
        pty = "python3 -c '\nimport pty\n' \"$TAM\" \"$root/knowledge\" || fail \"pager\"\n"
        (root / "tests" / "run.sh").write_text(run_sh + pty, encoding="utf-8")
        findings = check_mod.check_repo(root)
        self.assertEqual(len(findings), 1, findings)
        self.assert_finding_details(findings[0], "tests/run.sh", 5, "Pager fixtures")

        # A fixture root, or a marked exception, passes.
        (root / "tests" / "run.sh").write_text(
            run_sh + pty.replace('"$root/knowledge"', '"$root/tests/fixtures/guide"'),
            encoding="utf-8",
        )
        self.assertEqual(check_mod.check_repo(root), [])
        (root / "tests" / "run.sh").write_text(
            run_sh + pty.replace("import pty\n", "import pty\n# live-corpus: checks the installed index\n"),
            encoding="utf-8",
        )
        self.assertEqual(check_mod.check_repo(root), [])


if __name__ == "__main__":
    unittest.main()
