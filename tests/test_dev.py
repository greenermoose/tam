"""Behavioral and fixture tests for tools/dev.py (TCK11)."""

import importlib.util
import json
import os
from pathlib import Path
import pty
import select
import shutil
import subprocess
import sys
import tempfile
import unittest

DEV_SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "dev.py"
BUILDER = Path(__file__).resolve().parents[1] / "tools" / "build-index.py"

SPEC = importlib.util.spec_from_file_location("tam_dev", DEV_SCRIPT)
dev_mod = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(dev_mod)


ROADMAP_TEMPLATE = """# tam Roadmap

**Reviewed:** 2026-10-20. **Owner:** Fred (developer).
**Current Command Version:** `0.32.0`. **Current Corpus Version:** `0.33.6`.

## 2. Delivered history

| Version | Milestone / Focus | Status | Key Delivered Capabilities |
| :--- | :--- | :--- | :--- |
| **0.32.0 command / 0.33.5 corpus** | Home-row pager keys | Done (2026-10-20) | Home-row navigation. |
| **0.33.6 corpus** | Guide root index wording | Done (2026-10-20) | Retitled Guide root index. |


## 3. Current state and follow-up

| Branch | Command / corpus | Meaning |
| --- | --- | --- |
| `develop` | 0.32.0 / 0.33.6 | Newest source; installed only in the developer's `~/.local/dev` prefix. |
"""

DEVELOPMENT_TEMPLATE = """# tam development plan

## Decision record

Entries are dated historical decisions; the current sections and roadmap above
resolve superseded schedules. Do not execute a former plan from this history.

- **2026-10-20:** Existing decision entry.
"""

VERSION_H_TEMPLATE = """#ifndef TAM_VERSION_H
#define TAM_VERSION_H

#define TAM_COMMAND_NAME            "tam"
#define TAM_COMMAND_VERSION         "0.32.0"

#endif
"""

RUN_SH_TEMPLATE = """#!/bin/sh
set -eu
"$TAM" --version | grep -q '^tam command 0.32.0' || exit 1
"""

FIXTURE_MAKEFILE = """PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: all command corpus test libtam-check install

all: command corpus

libtam-check:
	@true

command:
	@mkdir -p build
	@printf '#!/bin/sh\\necho "tam command 0.32.0"\\n' > build/tam
	@chmod 755 build/tam

corpus:
	@python3 tools/build-index.py

test:
	@if [ -f fail_test ]; then seq 1 50; exit 1; fi
	@echo "fixture tests ok"

install: command corpus
	@mkdir -p $(BINDIR)
	@cp build/tam $(BINDIR)/tam
	@chmod 755 $(BINDIR)/tam
"""


def sample_page(title, track="", parent=None, crumbs=()):
    meta = [f'<meta name="track" content="{track}">']
    if parent is not None:
        meta.append(f'<meta name="parent" content="{parent}">')
    links = ['<a href="index.html">Guide</a>']
    links.extend(f'<a href="{p}">{lbl}</a>' for p, lbl in crumbs)
    nav = "" if title == "Guide" else (
        '<nav aria-label="Breadcrumb">' + " / ".join(links) + " / " + title + "</nav>"
    )
    return (
        f"<!doctype html><html><head><title>{title}</title>"
        f'<meta name="topic" content="{title.lower()}">'
        f'<meta name="summary" content="{title} summary">'
        f'<meta name="level" content="beginner">'
        + "".join(meta)
        + f"</head><body>{nav}<h1>{title}</h1></body></html>"
    )


class DevLoopTest(unittest.TestCase):
    def make_fixture(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        workspace = Path(temp.name)
        origin = workspace / "origin.git"
        root = workspace / "tam"
        registry = workspace / "coordination"
        home = workspace / "home"
        dev_prefix = home / ".local" / "dev"
        (dev_prefix / "bin").mkdir(parents=True)

        subprocess.run(["git", "init", "--bare", "-q", str(origin)], check=True)
        root.mkdir()
        (root / "knowledge" / "use").mkdir(parents=True)
        (root / "src").mkdir()
        (root / "tests").mkdir()
        (root / "tools").mkdir()
        (root / "docs" / "plans").mkdir(parents=True)

        (root / "Makefile").write_text(FIXTURE_MAKEFILE, encoding="utf-8")
        (root / "ROADMAP.md").write_text(ROADMAP_TEMPLATE, encoding="utf-8")
        (root / "README.md").write_text("# tam\n", encoding="utf-8")
        (root / "docs" / "plans" / "development.md").write_text(
            DEVELOPMENT_TEMPLATE, encoding="utf-8"
        )
        (root / "src" / "version.h").write_text(VERSION_H_TEMPLATE, encoding="utf-8")
        (root / "src" / "tam.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
        (root / "tests" / "run.sh").write_text(RUN_SH_TEMPLATE, encoding="utf-8")
        (root / "knowledge" / "version").write_text("0.33.6\n", encoding="utf-8")
        (root / "knowledge" / "index.html").write_text(
            sample_page("Guide"), encoding="utf-8"
        )
        (root / "knowledge" / "use" / "index.html").write_text(
            sample_page("Using", "use", "index.html"), encoding="utf-8"
        )
        (root / ".gitignore").write_text("build/\nknowledge/.build_id\n__pycache__/\n", encoding="utf-8")
        shutil.copyfile(BUILDER, root / "tools" / "build-index.py")
        shutil.copyfile(DEV_SCRIPT, root / "tools" / "dev.py")

        self.git(root, "init", "-q", "-b", "develop")
        self.git(root, "remote", "add", "origin", str(origin))
        self.git(root, "add", ".")
        self.git(root, "commit", "-qm", "Initial commit")
        subprocess.run(
            [sys.executable, str(root / "tools" / "build-index.py"), "--root", str(root)],
            check=True,
            capture_output=True,
        )
        self.git(root, "add", "knowledge")
        self.git(root, "commit", "-qm", "Initial indexes")
        self.git(root, "push", "-q", "-u", "origin", "develop")

        env = {
            **os.environ,
            "HOME": str(home),
            "PATH": f"{dev_prefix / 'bin'}:{os.environ.get('PATH', '')}",
            "TAM_WORK_WORKSPACE": str(workspace),
            "TAM_WORK_REGISTRY": str(registry),
            "GIT_CONFIG_NOSYSTEM": "1",
            "GIT_CONFIG_GLOBAL": os.devnull,
            "GIT_AUTHOR_NAME": "Fred Horch",
            "GIT_AUTHOR_EMAIL": "fred@example.invalid",
            "GIT_COMMITTER_NAME": "Fred Horch",
            "GIT_COMMITTER_EMAIL": "fred@example.invalid",
        }
        return root, env

    def git(self, root, *args):
        res = subprocess.run(
            [
                "git",
                "-c", "user.name=Fred Horch",
                "-c", "user.email=fred@example.invalid",
                "-c", "commit.gpgsign=false",
                *args,
            ],
            cwd=root,
            capture_output=True,
            text=True,
            check=True,
        )
        return res.stdout.strip()

    def run_dev_pty(self, root, env, inputs, extra_args=()):
        master_fd, slave_fd = pty.openpty()
        proc = subprocess.Popen(
            [sys.executable, str(DEV_SCRIPT), "--root", str(root), *extra_args],
            stdin=slave_fd,
            stdout=slave_fd,
            stderr=slave_fd,
            cwd=root,
            env=env,
            close_fds=True,
        )
        os.close(slave_fd)
        feed = list(inputs)
        output = bytearray()
        prompts = (
            b"[c]ontinue, [d]iff, [q]uit",
            b"[p]atch (default) / [m]inor:",
            b"One-line summary:",
            b"Commit subject",
            b"Commit body (optional):",
            b"Install prefix",
            b"Type 'yes' to confirm:",
            b"Push develop? [y/N]:",
        )
        sent_for_pos = 0
        try:
            while True:
                rlist, _, _ = select.select([master_fd], [], [], 10.0)
                if not rlist:
                    break
                try:
                    chunk = os.read(master_fd, 4096)
                except OSError:
                    break
                if not chunk:
                    break
                output.extend(chunk)
                tail = bytes(output[sent_for_pos:])
                if feed and any(p in tail for p in prompts):
                    line = feed.pop(0) + "\n"
                    os.write(master_fd, line.encode("utf-8"))
                    sent_for_pos = len(output)
        finally:
            os.close(master_fd)
        rc = proc.wait(timeout=15)
        return rc, output.decode("utf-8", errors="replace")

    def test_refuses_without_terminal_off_develop_or_during_rebase(self):
        root, env = self.make_fixture()
        res = subprocess.run(
            [sys.executable, str(DEV_SCRIPT), "--root", str(root)],
            cwd=root,
            env=env,
            capture_output=True,
            text=True,
        )
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("stdin and stdout must be terminals", res.stderr)
        self.assertIn("Nothing was committed.", res.stderr)

        self.git(root, "checkout", "-qb", "feature")
        rc, out = self.run_dev_pty(root, env, [])
        self.assertNotEqual(rc, 0)
        self.assertIn("must be on branch 'develop'", out)

        self.git(root, "checkout", "-q", "develop")
        rebase_dir = root / ".git" / "rebase-merge"
        rebase_dir.mkdir()
        rc, out = self.run_dev_pty(root, env, [])
        self.assertNotEqual(rc, 0)
        self.assertIn("rebase in progress", out)
        rebase_dir.rmdir()

    def test_collect_groups_lanes_filters_generated_and_excludes_claimed_paths(self):
        root, env = self.make_fixture()
        claim_env = {
            **env,
            "TAM_WORK_OWNER": "claude",
            "TAM_WORK_SESSION": "other-session-1",
        }
        subprocess.run(
            ["tam-work", "begin", "--scope", "src/tam.c", "--note", "agent work"],
            cwd=root,
            env=claim_env,
            check=True,
            capture_output=True,
        )

        # Edit corpus, command (one claimed, one unclaimed), other, and generated paths
        (root / "knowledge" / "use" / "index.html").write_text(
            sample_page("Using", "use", "index.html").replace("Using summary", "Updated using"),
            encoding="utf-8",
        )
        (root / "src" / "tam.c").write_text("int main(void) { return 1; }\n", encoding="utf-8")
        (root / "src" / "helper.c").write_text("int helper(void) { return 0; }\n", encoding="utf-8")
        (root / "README.md").write_text("# tam updated\n", encoding="utf-8")
        (root / "knowledge" / "index.tsv").write_text("dirty generated\n", encoding="utf-8")
        (root / "knowledge" / "coverage.json").write_text("{}\n", encoding="utf-8")
        (root / "knowledge" / "index.skills.tsv").write_text("path\tskills\n", encoding="utf-8")
        (root / "knowledge" / ".build_id").write_text("dirty-id\n", encoding="utf-8")
        (root / "build").mkdir(exist_ok=True)
        (root / "build" / "tam").write_text("binary", encoding="utf-8")
        (root / "tools" / "__pycache__").mkdir(exist_ok=True)
        (root / "tools" / "__pycache__" / "x.pyc").write_bytes(b"123")

        res = subprocess.run(
            [sys.executable, str(DEV_SCRIPT), "--root", str(root), "--dry-run"],
            cwd=root,
            env=env,
            capture_output=True,
            text=True,
        )
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertIn("Corpus (knowledge/):\n  knowledge/use/index.html", res.stdout)
        self.assertIn("Command (src/):\n  src/helper.c (untracked)", res.stdout)
        self.assertIn("Other:\n  README.md", res.stdout)
        self.assertIn("Excluded paths (claimed by another session):", res.stdout)
        self.assertIn("src/tam.c", res.stdout)
        self.assertIn("owner claude", res.stdout)
        self.assertNotIn("knowledge/index.tsv", res.stdout)
        self.assertNotIn("knowledge/coverage.json", res.stdout)
        self.assertNotIn("knowledge/index.skills.tsv", res.stdout)
        self.assertNotIn("knowledge/.build_id", res.stdout)
        self.assertNotIn("build/tam", res.stdout)
        self.assertNotIn("__pycache__", res.stdout)
        self.assertIn("Corpus version: 0.33.6 -> 0.33.7 (patch, default) or 0.34.0 (minor)", res.stdout)
        self.assertIn("Command version: 0.32.0 -> 0.32.1 (patch, default) or 0.33.0 (minor)", res.stdout)

    def test_version_replacements_apply_exactly_once_and_fail_loudly_on_mismatch(self):
        root, _ = self.make_fixture()
        dev_mod.apply_corpus_version_edits(
            root, "0.33.6", "0.33.7", "Clarify using index", "2026-10-20"
        )
        self.assertEqual((root / "knowledge" / "version").read_text().strip(), "0.33.7")
        roadmap = (root / "ROADMAP.md").read_text()
        self.assertIn("**Current Corpus Version:** `0.33.7`.", roadmap)
        self.assertIn("| `develop` | 0.32.0 / 0.33.7 |", roadmap)
        self.assertIn(
            "| **0.33.7 corpus** | Clarify using index | Done (2026-10-20) | Clarify using index |",
            roadmap,
        )

        dev_mod.apply_command_version_edits(
            root, "0.32.0", "0.33.0", "minor", "Add helper command", "2026-10-20"
        )
        self.assertIn('"0.33.0"', (root / "src" / "version.h").read_text())
        self.assertIn("^tam command 0.33.0", (root / "tests" / "run.sh").read_text())
        roadmap = (root / "ROADMAP.md").read_text()
        self.assertIn("**Current Command Version:** `0.33.0`.", roadmap)
        self.assertIn("| `develop` | 0.33.0 / 0.33.7 |", roadmap)
        self.assertIn(
            "| **0.33.0 command** | Add helper command | Done (2026-10-20) | Add helper command |",
            roadmap,
        )
        dev_md = (root / "docs" / "plans" / "development.md").read_text()
        self.assertIn(
            "- **2026-10-20:** Command 0.33.0 (minor): Add helper command",
            dev_md,
        )

        # Calling again with old version fails loudly because the pattern no longer matches
        with self.assertRaises(dev_mod.DevError):
            dev_mod.apply_corpus_version_edits(
                root, "0.33.6", "0.33.8", "Duplicate bump", "2026-10-20"
            )
        with self.assertRaises(dev_mod.DevError):
            dev_mod.apply_command_version_edits(
                root, "0.32.0", "0.33.1", "patch", "Duplicate bump", "2026-10-20"
            )

    def test_failed_test_keeps_version_bump_and_rerun_detects_already_bumped(self):
        root, env = self.make_fixture()
        page_path = root / "knowledge" / "use" / "index.html"
        page_path.write_text(
            page_path.read_text().replace("Using summary", "Updated summary"),
            encoding="utf-8",
        )
        (root / "fail_test").write_text("fail\n", encoding="utf-8")

        # First run: test fails in step 5; nothing committed, version edits stay
        rc, out = self.run_dev_pty(
            root,
            env,
            ["c", "p", "Refine using guide"],
        )
        self.assertNotEqual(rc, 0)
        self.assertIn("make test failed", out)
        self.assertIn("Nothing was committed.", out)
        self.assertEqual((root / "knowledge" / "version").read_text().strip(), "0.33.7")

        # Claim must have been released on failure
        claims = json.loads(
            subprocess.run(
                ["tam-work", "status", "--json", "--repo", "tam"],
                cwd=root,
                env=env,
                capture_output=True,
                text=True,
                check=True,
            ).stdout
        )
        self.assertEqual(claims, [])

        # Fix test failure and rerun: step 4 detects "already bumped to 0.33.7"
        (root / "fail_test").unlink()
        rc2, out2 = self.run_dev_pty(
            root,
            env,
            ["c", "", "", "", "y"],
        )
        self.assertEqual(rc2, 0, out2)
        self.assertIn("Corpus: already bumped to 0.33.7", out2)
        self.assertIn("Refresh corpus index revisions after", out2)
        self.assertIn("Pushed develop to origin.", out2)

        log_subjects = self.git(root, "log", "-n", "2", "--format=%s").splitlines()
        self.assertTrue(log_subjects[0].startswith("Refresh corpus index revisions after "))
        self.assertEqual(log_subjects[1], "Refine using guide; corpus 0.33.7")

    def test_install_prefix_guards_and_upstream_ahead_push_skip(self):
        root, env = self.make_fixture()
        page_path = root / "knowledge" / "use" / "index.html"
        page_path.write_text(
            page_path.read_text().replace("Using summary", "Another update"),
            encoding="utf-8",
        )

        # Advance origin/develop from a second clone so upstream is ahead
        other_clone = root.parent / "other-clone"
        subprocess.run(["git", "clone", "-q", "-b", "develop", str(root.parent / "origin.git"), str(other_clone)], check=True)
        (other_clone / "README.md").write_text("# upstream change\n", encoding="utf-8")
        self.git(other_clone, "add", "README.md")
        self.git(other_clone, "commit", "-qm", "Upstream commit")
        self.git(other_clone, "push", "-q", "origin", "develop")

        # Try /usr/local prefix -> refused after commit, reporting what WAS committed
        rc, out = self.run_dev_pty(
            root,
            env,
            ["c", "p", "Test prefix guard", "", "", "/usr/local"],
        )
        self.assertNotEqual(rc, 0)
        self.assertIn("refusing to install to /usr/local", out)
        self.assertIn("Committed before failure:", out)

        # Now edit again and install to a custom prefix (requiring 'yes'), verifying upstream push skip
        page_path.write_text(
            page_path.read_text().replace("Another update", "Third update"),
            encoding="utf-8",
        )
        custom_prefix = root.parent / "custom-prefix"
        (custom_prefix / "bin").mkdir(parents=True)
        custom_env = {
            **env,
            "PATH": f"{custom_prefix / 'bin'}:{env['PATH']}",
        }
        rc2, out2 = self.run_dev_pty(
            root,
            custom_env,
            ["c", "p", "Custom prefix install", "", "", str(custom_prefix), "yes"],
        )
        self.assertEqual(rc2, 0, out2)
        self.assertIn("Upstream has commits not in HEAD; not pushing:", out2)
        self.assertIn("Push state: not pushed (upstream has new commits)", out2)


class FailureExplanationTest(unittest.TestCase):
    PAGE_CHANGES = {
        "corpus": ["knowledge/index.html", "knowledge/version"],
        "command": [],
        "other": ["ROADMAP.md"],
    }
    COMMAND_TEST_OUTPUT = (
        "cc -o build/tam src/tam.c\n"
        "python3 tools/build-index.py\n"
        "tests/run.sh /work/tam/build/tam\n"
        "ok - show subcommand\n"
        "AssertionError: expected info:dir, got learn/documentation.html\n"
        "FAIL: interactive pager link navigation failed\n"
        "make: *** [Makefile:115: test] Error 1\n"
    )
    PAGE_CHECK_OUTPUT = (
        "tests/run.sh /work/tam/build/tam\n"
        "All tam command tests passed.\n"
        "python3 tools/lint-corpus.py\n"
        "knowledge/use/index.html:12: unclosed <li>\n"
        "make: *** [Makefile:118: test] Error 1\n"
    )

    def test_page_only_change_failing_a_command_test_points_to_an_agent(self):
        lines = dev_mod.explain_test_failure(self.COMMAND_TEST_OUTPUT, self.PAGE_CHANGES)
        text = "\n".join(lines)
        self.assertEqual(lines[0], "FAIL: interactive pager link navigation failed")
        self.assertIn("Failing check: tests/run.sh /work/tam/build/tam", text)
        self.assertIn("Your pages are probably fine. Ask an agent", text)

    def test_page_check_failure_asks_for_a_page_fix(self):
        lines = dev_mod.explain_test_failure(self.PAGE_CHECK_OUTPUT, self.PAGE_CHANGES)
        text = "\n".join(lines)
        self.assertIn("Failing check: python3 tools/lint-corpus.py", text)
        self.assertIn("Fix the page it names", text)
        self.assertNotIn("Ask an agent", text)

    def test_command_change_only_names_the_check(self):
        changes = {"corpus": [], "command": ["src/pager.c"], "other": []}
        lines = dev_mod.explain_test_failure(self.COMMAND_TEST_OUTPUT, changes)
        text = "\n".join(lines)
        self.assertIn("Failing check: tests/run.sh", text)
        self.assertNotIn("Ask an agent", text)
        self.assertNotIn("Fix the page", text)

    def test_build_failure_before_tests(self):
        output = "cc -o build/tam src/tam.c\nsrc/tam.c:1: error\nmake: *** [Makefile:90: build/tam] Error 1\n"
        lines = dev_mod.explain_test_failure(output, self.PAGE_CHANGES)
        self.assertIn("The build failed before the tests ran.", lines)
        self.assertIn("Ask an agent", "\n".join(lines))


if __name__ == "__main__":
    unittest.main()
