"""Focused compatibility and hierarchy checks for tools/build-index.py."""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

BUILDER = Path(__file__).resolve().parents[1] / "tools" / "build-index.py"
MAKEFILE = BUILDER.parent.parent / "Makefile"
INDEXES = ("index.tsv", "index.json", "index.parents.tsv", "index.kinds.tsv",
           "index.skills.tsv", "coverage.json")


def page(title, track="", parent=None, crumbs=(), prereqs=None):
    metadata = [f'<meta name="track" content="{track}">']
    if parent is not None:
        metadata.append(f'<meta name="parent" content="{parent}">')
    if prereqs is not None:
        metadata.append(f'<meta name="prereqs" content="{prereqs}">')
    links = ['<a href="index.html">Guide</a>']
    links.extend(f'<a href="{path}">{label}</a>' for path, label in crumbs)
    nav = "" if title == "Guide" else (
        '<nav aria-label="Breadcrumb">' + " / ".join(links) + " / " + title + "</nav>"
    )
    return ("<!doctype html><html><head><title>" + title + "</title>" +
            "<meta name=\"topic\" content=\"" + title.lower().replace(" ", "-") + "\">" +
            "<meta name=\"summary\" content=\"" + title + " summary\">" +
            "<meta name=\"level\" content=\"beginner\">" + "".join(metadata) +
            "</head><body>" + nav + "<h1>" + title + "</h1></body></html>")


class BuildIndex(unittest.TestCase):
    def register_skills(self, root, cells=None):
        path = root / "knowledge/learn/skills/skills.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        cells = cells if cells is not None else [
            {"id": "shell.beginner", "domain": "shell", "level": "beginner", "can_do": "Connect commands"},
            {"id": "shell.expert", "domain": "shell", "level": "expert", "can_do": "Explain descriptors"}]
        path.write_text(json.dumps({"version": 1, "cells": cells}))
        return path

    def test_learning_sidecars_and_truthful_page_path_gaps(self):
        text = page("Guide").replace("</head>", '<meta name="kind" content="tutorial"><meta name="skills" content="shell.beginner"></head>')
        root = self.fixture("0.39.0", {"index.html": text})
        self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        entry = self.index(root)["index.html"]
        self.assertEqual(entry["kind"], "tutorial")
        self.assertEqual(entry["skills"], ["shell.beginner"])
        self.assertEqual(len((root / "knowledge/index.tsv").read_text().splitlines()[0].split("\t")), 10)
        report = json.loads((root / "knowledge/coverage.json").read_text())
        self.assertEqual(report["cells"][0]["pages"], ["index.html"])
        self.assertTrue(report["cells"][0]["path_gap"])
        self.assertTrue(report["cells"][1]["page_gap"])
        self.assertEqual((root / "knowledge/index.kinds.tsv").read_text(), "path\tkind\nindex.html\ttutorial\n")
        (root / "knowledge/index.html").write_text(text.replace('content="tutorial"', 'content="path"'))
        self.assertEqual(self.build(root).returncode, 0)
        report = json.loads((root / "knowledge/coverage.json").read_text())
        self.assertEqual(report["cells"][0]["paths"], ["index.html"])
        self.assertTrue(report["cells"][0]["page_gap"])
        self.assertFalse(report["cells"][0]["path_gap"])

    def test_unknown_and_wrong_level_skills_do_not_replace_good_outputs(self):
        root = self.fixture("0.39.0", {"index.html": page("Guide")})
        self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        before = self.outputs(root)
        cases = [('kind', 'article', 'unknown kind'),
                 ('skills', 'missing.beginner', 'unknown skill ID'),
                 ('skills', 'shell.expert', 'outside the declared page levels'),
                 ('skills', 'shell.beginner, shell.beginner', 'duplicate skill IDs')]
        for name, value, message in cases:
            with self.subTest(value=value):
                (root / "knowledge/index.html").write_text(page("Guide").replace('</head>', f'<meta name="{name}" content="{value}"></head>'))
                result = self.build(root)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn(message, result.stderr)
                self.assertEqual(before, self.outputs(root))

    def test_registry_errors_are_plain_and_bounded(self):
        root = self.fixture("0.39.0", {"index.html": page("Guide")})
        path = self.register_skills(root)
        for content in ('[]', '{', '{"version": 2, "cells": []}',
                        '{"version":1,"cells":[null]}'):
            with self.subTest(content=content):
                path.write_text(content)
                result = self.build(root)
                self.assertEqual(result.returncode, 1)
                self.assertIn('skills.json:', result.stderr)
                self.assertNotIn('Traceback', result.stderr)

    def test_glossary_and_bibliography_links_resolve_structurally(self):
        docs = {
            'index.html': page('Guide').replace('</body>', '<dfn><a href="learn/glossary/index.html">stream</a></dfn><cite data-ref="bash"><a href="learn/bibliography/index.html">Bash</a></cite></body>'),
            'learn/index.html': page('Learning', 'learn', 'index.html'),
            'learn/glossary/index.html': page('Glossary', 'learn', 'learn/index.html', [('learn/index.html', 'Learning')]).replace('</body>', '<p id="stream">Bytes</p></body>'),
            'learn/bibliography/index.html': page('Bibliography', 'learn', 'learn/index.html', [('learn/index.html', 'Learning')]).replace('</body>', '<section class="reference" id="bash">Bash</section></body>'),
        }
        root = self.fixture('0.39.0', docs)
        self.assertEqual(self.build(root).returncode, 0)
        bad = [('<dfn>stream</dfn>', 'dfn requires a glossary link'),
               ('<dfn><a href="learn/glossary/absent.html">term</a></dfn>', 'missing glossary target'),
               ('<dfn><a href="learn/glossary/index.html#stream">term</a></dfn>', 'cross-page glossary fragments'),
               ('<cite data-ref="absent">Manual</cite>', 'unknown bibliography ID'),
               ('<cite data-ref="bash">Bash</cite>', 'requires its bibliography link')]
        for snippet, error in bad:
            with self.subTest(snippet=snippet):
                (root / 'knowledge/index.html').write_text(page('Guide').replace('</body>', snippet+'</body>'))
                result = self.build(root)
                self.assertEqual(result.returncode, 1)
                self.assertIn(error, result.stderr)

    def test_anatomy_gaps_warn_without_breaking_legacy_build(self):
        text = page('Guide').replace('</body>', '<section data-level="beginner"><h2>Exercise</h2><p>Try something</p></section></body>')
        root = self.fixture('0.39.0', {'index.html': text})
        result = self.build(root)
        self.assertEqual(result.returncode, 0)
        report = json.loads((root / 'knowledge/coverage.json').read_text())
        self.assertIn('quiz', report['warnings'][0]['missing'])
        self.assertIn('index.html', report['unclassified_pages'])
        self.assertIn('index.html', report['unmapped_pages'])
        self.assertIn('Learning warnings: 1', result.stdout)
        self.assertEqual((root/'knowledge/index.kinds.tsv').read_text(), 'path\tkind\n')
        self.assertEqual((root/'knowledge/index.skills.tsv').read_text(), 'path\tskills\n')

    def test_completed_part_promotes_only_its_anatomy_warnings_to_errors(self):
        text = page('Using', 'use', 'index.html').replace('</body>', '<section data-level="beginner"><h2>Task</h2><p>Partial lesson</p></section></body>')
        root = self.fixture('0.40.0', {'index.html': page('Guide'), 'use/index.html': text})
        registry = self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        before = self.outputs(root)
        data = json.loads(registry.read_text())
        data['enforce_tracks'] = ['use']
        registry.write_text(json.dumps(data))
        result = self.build(root)
        self.assertEqual(result.returncode, 1)
        self.assertIn('use/index.html: beginner anatomy:', result.stderr)
        self.assertIn('quiz', result.stderr)
        self.assertEqual(before, self.outputs(root))
        data['enforce_tracks'] = ['maintain']
        registry.write_text(json.dumps(data))
        self.assertEqual(self.build(root).returncode, 0)

    def test_path_skills_span_visited_levels_but_page_skills_do_not(self):
        text = page('Guide').replace('</head>', '<meta name="kind" content="path"><meta name="skills" content="shell.expert"></head>')
        root = self.fixture('0.40.0', {'index.html': text})
        self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        report = json.loads((root/'knowledge/coverage.json').read_text())
        self.assertEqual(report['cells'][1]['paths'], ['index.html'])
        (root/'knowledge/index.html').write_text(text.replace('content="path"','content="tutorial"'))
        self.assertIn('outside the declared page levels', self.build(root).stderr)

    def test_invalid_enforcement_policies_fail_plainly(self):
        root = self.fixture('0.40.0', {'index.html': page('Guide')})
        registry = self.register_skills(root)
        data = json.loads(registry.read_text())
        for value in ('use', ['unknown'], ['use','use'], [1]):
            data['enforce_tracks'] = value
            registry.write_text(json.dumps(data))
            result = self.build(root)
            self.assertEqual(result.returncode, 1)
            self.assertIn('enforce_tracks', result.stderr)
            self.assertNotIn('Traceback', result.stderr)

    def fixture(self, version, documents):
        temp = tempfile.TemporaryDirectory()
        root = Path(temp.name)
        knowledge = root / "knowledge"
        knowledge.mkdir()
        (knowledge / "version").write_text(version + "\n")
        for rel, text in documents.items():
            path = knowledge / rel
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
        self.addCleanup(temp.cleanup)
        return root

    def build(self, root):
        return subprocess.run([sys.executable, str(BUILDER), "--root", str(root)],
                              capture_output=True, text=True, timeout=30)

    def make_fixture(self):
        root = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use", "index.html"),
        })
        shutil.copyfile(MAKEFILE, root / "Makefile")
        (root / "tools").mkdir()
        shutil.copyfile(BUILDER, root / "tools/build-index.py")
        (root / ".gitignore").write_text("knowledge/.build_id\n")
        return root

    def make(self, root, *args):
        return subprocess.run(["make", "--no-print-directory", "-s", *args],
                              cwd=root, capture_output=True, text=True, timeout=30,
                              env={**os.environ, "LC_ALL": "C", "MAKEFLAGS": "", "MFLAGS": ""})

    def make_ok(self, root, *args):
        result = self.make(root, *args)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def git(self, root, *args, date="2026-10-18T12:00:00+00:00"):
        result = subprocess.run(
            ["git", "-c", "user.name=Corpus Test", "-c", "user.email=test@example.invalid",
             "-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null", *args],
            cwd=root, capture_output=True, text=True, timeout=30,
            env={**os.environ, "GIT_CONFIG_NOSYSTEM": "1", "GIT_CONFIG_GLOBAL": os.devnull,
                 "GIT_AUTHOR_DATE": date, "GIT_COMMITTER_DATE": date})
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result.stdout.strip()

    def index(self, root):
        return {item["path"]: item for item in json.loads(
            (root / "knowledge/index.json").read_text())}

    def outputs(self, root, names=INDEXES):
        return {name: ((root / "knowledge" / name).read_bytes(),
                       (root / "knowledge" / name).stat().st_mtime_ns) for name in names}

    def test_block_levels_validate_without_adding_ladder_sections(self):
        for tag in ("aside", "p", "li", "pre", "details", "div", "table"):
            with self.subTest(tag=tag):
                text = page("Guide").replace("</body>", f'<{tag} data-level="beginner">Shared</{tag}></body>')
                root = self.fixture("0.37.1", {"index.html": text})
                self.assertEqual(self.build(root).returncode, 0)
                for level, message in (("novice", "unknown data-level"), ("expert", "not declared")):
                    (root / "knowledge/index.html").write_text(text.replace('data-level="beginner"', f'data-level="{level}"'))
                    result = self.build(root)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn(message, result.stdout + result.stderr)

    def test_layered_pages_and_ladders(self):
        shared = page("Guide").replace('content="beginner"', 'content="beginner, intermediate, expert"')
        body = ('<aside data-level="beginner">Hint</aside>'
                '<p data-level="intermediate">Model</p>'
                '<table data-level="expert"><tr><td>Reference</td></tr></table>'
                '<section class="quiz" data-level="beginner"><h3>Quiz</h3></section>')
        root = self.fixture("0.37.1", {"index.html": shared.replace("</body>", body + "</body>")})
        self.assertEqual(self.build(root).returncode, 0)
        ladder = ''.join(f'<section data-level="{level}"><h2>{level}</h2></section>'
                         for level in ("beginner", "intermediate", "expert"))
        target = root / "knowledge/index.html"
        target.write_text(shared.replace("</body>", ladder + body + "</body>"))
        self.assertEqual(self.build(root).returncode, 0)
        for invalid in (ladder.replace('data-level="expert"', 'data-level="beginner"'), ""):
            target.write_text(shared.replace("</body>", invalid + "</body>"))
            result = self.build(root)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("matching sections", result.stdout + result.stderr)

    def test_make_rechecks_edits_and_page_inventory_at_any_depth(self):
        root = self.make_fixture()
        self.make_ok(root, "corpus")
        path = root / "knowledge/use/index.html"
        timestamps = path.stat()
        path.write_text(path.read_text().replace("Using summary", "Edited summary"))
        os.utime(path, ns=(timestamps.st_atime_ns, timestamps.st_mtime_ns))
        self.make_ok(root, "corpus")
        self.assertEqual(self.index(root)["use/index.html"]["summary"], "Edited summary")

        deep = root / "knowledge/use/deep/nested/task.html"
        deep.parent.mkdir(parents=True)
        deep.write_text(page("Task", "use", "use/index.html",
                             crumbs=(("use/index.html", "Using"),)))
        self.make_ok(root, "corpus")
        self.assertIn("use/deep/nested/task.html", self.index(root))
        renamed = deep.with_name("renamed.html")
        deep.rename(renamed)
        self.make_ok(root, "corpus")
        self.assertNotIn("use/deep/nested/task.html", self.index(root))
        self.assertIn("use/deep/nested/renamed.html", self.index(root))
        renamed.unlink()
        self.make_ok(root, "corpus")
        self.assertNotIn("use/deep/nested/renamed.html", self.index(root))

    def test_make_refreshes_git_metadata_and_dirty_marker_after_commits(self):
        root = self.make_fixture()
        self.git(root, "init", "-q")
        self.git(root, "add", ".")
        self.git(root, "commit", "-qm", "Initial pages")
        self.make_ok(root, "corpus")
        self.git(root, "add", "knowledge")
        self.git(root, "commit", "-qm", "Record indexes")
        self.make_ok(root, "corpus")
        marker = root / "knowledge/.build_id"
        self.assertEqual(marker.read_text().strip(), self.git(root, "rev-parse", "--short", "HEAD"))

        path = root / "knowledge/use/index.html"
        path.write_text(path.read_text().replace("Using summary", "New summary"))
        self.make_ok(root, "corpus")
        self.assertTrue(marker.read_text().strip().endswith("-dirty"))
        before = path.stat().st_mtime_ns
        self.git(root, "add", "knowledge/use/index.html")
        self.git(root, "commit", "-qm", "Edit a page", date="2026-10-19T13:45:00+00:00")
        self.assertEqual(path.stat().st_mtime_ns, before)
        self.make_ok(root, "corpus")
        entry = self.index(root)["use/index.html"]
        self.assertEqual(entry["rev"], 2)
        self.assertEqual(entry["updated"], "2026-10-19 13:45")
        # Rewritten indexes themselves keep the build dirty until committed.
        self.assertTrue(marker.read_text().strip().endswith("-dirty"))
        self.git(root, "add", "knowledge")
        self.git(root, "commit", "-qm", "Refresh indexes")
        self.make_ok(root, "corpus")
        self.assertFalse(marker.read_text().strip().endswith("-dirty"))
        self.assertEqual(self.git(root, "status", "--porcelain"), "")

        names = (*INDEXES, ".build_id")
        stable = self.outputs(root, names)
        self.make_ok(root, "corpus")
        self.assertEqual(self.outputs(root, names), stable)
        for name in names:
            with self.subTest(missing=name):
                (root / "knowledge" / name).unlink()
                self.make_ok(root, "corpus")
                self.assertEqual((root / "knowledge" / name).read_bytes(), stable[name][0])

    def test_unchanged_archive_build_preserves_outputs_without_git(self):
        root = self.make_fixture()
        self.make_ok(root, "corpus")
        # Pin mtimes to the past so even coarse filesystems detect a rewrite.
        for name in INDEXES:
            os.utime(root / "knowledge" / name, ns=(1_000_000_000, 1_000_000_000))
        before = self.outputs(root)
        self.make_ok(root, "corpus")
        self.assertEqual(self.outputs(root), before)
        self.assertFalse((root / "knowledge/.build_id").exists())

    def test_parallel_and_direct_index_targets_share_one_generation(self):
        root = self.make_fixture()
        targets = [f"knowledge/{name}" for name in INDEXES]
        result = self.make_ok(root, "-j8", "corpus", *targets)
        self.assertEqual(result.stdout.count("Generated index.tsv"), 1)
        for target in targets:
            (root / target).unlink()
            result = self.make_ok(root, target)
            self.assertTrue((root / target).is_file())
            self.assertEqual(result.stdout.count("Generated index.tsv"), 1)

    def test_invalid_metadata_fails_make_without_changing_outputs(self):
        root = self.make_fixture()
        self.make_ok(root, "corpus")
        before = self.outputs(root)
        path = root / "knowledge/use/index.html"
        path.write_text(path.read_text().replace('content="index.html"', 'content="missing.html"'))
        result = self.make(root, "corpus")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("does not name an existing page", result.stderr)
        self.assertEqual(self.outputs(root), before)

    def test_corpus_and_test_regenerates_before_checks(self):
        root = self.make_fixture()
        # Isolate corpus ordering from reader compilation and its dependencies.
        # Keep the real corpus-and-test recipe, with lightweight test fixtures.
        (root / "tests").mkdir()
        (root / "tests/__init__.py").write_text("")
        for name in ("test_lint_corpus.py", "test_build_index.py"):
            (root / "tests" / name).write_text(
                "from pathlib import Path\nimport unittest\n"
                "class Check(unittest.TestCase):\n"
                "    def test_generated_index_exists(self):\n"
                "        self.assertTrue(Path('knowledge/index.json').is_file())\n")
        (root / "tools/lint-corpus.py").write_text(
            "from pathlib import Path\n"
            "assert Path('knowledge/index.json').is_file()\n"
            "print('lint ran after generation')\n")
        guide = root / "tests/guide.sh"
        guide.write_text("#!/bin/sh\nset -eu\ntest -f knowledge/index.tsv\necho 'guide checked'\n")
        guide.chmod(0o755)
        result = self.make_ok(root, "-j8", "-o", "command", "corpus", "corpus-and-test")
        self.assertEqual(result.stdout.count("Generated index.tsv"), 1)
        self.assertLess(result.stdout.index("Generated index.tsv"),
                        result.stdout.index("lint ran after generation"))
        self.assertIn("guide checked", result.stdout)
        result = self.make(root, "test-corpus")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("No rule to make target", result.stderr)

        path = root / "knowledge/use/index.html"
        path.write_text(path.read_text().replace('content="index.html"', 'content="missing.html"'))
        result = self.make(root, "-o", "command", "corpus-and-test")
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("lint ran after generation", result.stdout)

    def test_five_tracks_and_hierarchy_are_additive_without_changing_tsv(self):
        tracks = ("install", "learn", "use", "maintain", "improve")
        docs = {"index.html": page("Guide")}
        for track in tracks:
            rel = f"{track}/index.html"
            docs[rel] = page(track.title(), track, "index.html")
        root = self.fixture("0.31.0", docs)
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)
        knowledge = root / "knowledge"
        rows = (knowledge / "index.tsv").read_text().splitlines()
        self.assertEqual(len(rows[0].split("\t")), 10)
        self.assertEqual(len(rows[1].split("\t")), 10)
        data = {item["path"]: item for item in json.loads((knowledge / "index.json").read_text())}
        self.assertTrue(all(data[f"{track}/index.html"]["track"] in tracks for track in tracks))
        self.assertEqual(data["maintain/index.html"]["parent"], "index.html")
        self.assertEqual(data["index.html"]["parent"], None)
        parents = (knowledge / "index.parents.tsv").read_text().splitlines()
        self.assertEqual(parents[0], "path\tparent")
        self.assertIn("maintain/index.html\tindex.html", parents)

    def test_old_three_track_corpus_uses_directory_fallback(self):
        docs = {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use"),
            "use/task.html": page("Task", "use", crumbs=(("use/index.html", "Using"),),
                                  prereqs="using"),
            "maintain/index.html": page("Maintaining", "maintain"),
            "improve/index.html": page("Improving", "improve"),
        }
        root = self.fixture("0.30.0", docs)
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)
        rows = (root / "knowledge/index.tsv").read_text().splitlines()
        self.assertEqual(rows[0].split("\t")[:10], [
            "topic", "path", "title", "summary", "keywords", "level", "track",
            "prereqs", "rev", "updated"])
        data = {item["path"]: item for item in json.loads(
            (root / "knowledge/index.json").read_text())}
        self.assertEqual(data["use/task.html"]["parent"], "use/index.html")
        self.assertEqual(data["use/task.html"]["topic"], "task")
        self.assertEqual(data["use/task.html"]["levels"], ["beginner"])
        self.assertEqual(data["use/task.html"]["prereqs"], ["using"])

    def test_topic_that_is_not_a_slug_is_rejected(self):
        root = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use", "index.html").replace(
                'content="using"', 'content="using tam"'),
        })
        result = self.build(root)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("topic 'using tam' is not a slug", result.stderr)

    def test_missing_and_cyclic_parents_are_rejected(self):
        missing = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use", "absent.html"),
        })
        result = self.build(missing)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("does not name an existing page", result.stderr)

        cyclic = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use", "use/task.html"),
            "use/task.html": page("Task", "use", "use/index.html"),
        })
        result = self.build(cyclic)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("contains a cycle", result.stderr)

    def test_parent_part_track_mismatch_is_rejected(self):
        root = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "maintain/index.html": page("Maintaining", "maintain", "index.html"),
            "use/task.html": page("Task", "improve", "maintain/index.html",
                                  crumbs=(("maintain/index.html", "Maintaining"),)),
        })
        result = self.build(root)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("does not match parent part", result.stderr)


if __name__ == "__main__":
    unittest.main()
