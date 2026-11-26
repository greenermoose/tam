"""Focused compatibility and hierarchy checks for tools/build-index.py."""

import contextlib
import io
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

BUILDER = Path(__file__).resolve().parents[1] / "tools" / "build-index.py"
COVERAGE_TOOL = (
    Path(__file__).resolve().parents[1] / "tools" / "report-component-coverage.py"
)
MAKEFILE = BUILDER.parent.parent / "Makefile"
INDEXES = ("index.tsv", "index.json", "index.parents.tsv", "index.kinds.tsv",
           "index.skills.tsv", "index.paths.tsv", "index.search.tsv",
           "index.anchors.tsv", "coverage.json")

spec = importlib.util.spec_from_file_location("build_index_tool", str(BUILDER))
build_index_mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build_index_mod)

SEARCH_TOKENS = BUILDER.parent / "search_tokens.py"
token_spec = importlib.util.spec_from_file_location("search_tokens", str(SEARCH_TOKENS))
search_tokens_mod = importlib.util.module_from_spec(token_spec)
token_spec.loader.exec_module(search_tokens_mod)


class BuildResult:
    def __init__(self, returncode, stdout, stderr):
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr


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
            "<meta name=\"level\" content=\"novice\">" + "".join(metadata) +
            "</head><body>" + nav + "<h1>" + title + "</h1></body></html>")


class BuildIndex(unittest.TestCase):
    def register_skills(self, root, cells=None):
        path = root / "knowledge/learn/skills/skills.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        cells = cells if cells is not None else [
            {"id": "shell.novice", "domain": "shell", "level": "novice", "can_do": "Connect commands"},
            {"id": "shell.expert", "domain": "shell", "level": "expert", "can_do": "Explain descriptors"}]
        path.write_text(json.dumps({"version": 1, "cells": cells}))
        return path

    def test_subject_hierarchy_and_escaped_search_terms(self):
        docs = {
            "index.html": page("Guide"),
            "security/index.html": page("Security", "security", "index.html"),
            "security/topic.html": page("Topic", "security", "security/index.html", [("security/index.html", "Security")]).replace('</head>', '<meta name="keywords" content="&gt;, A &amp; B"></head>'),
        }
        root = self.fixture("0.90.0", docs)
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)
        data = json.loads((root / "knowledge/index.json").read_text())
        topic = next(e for e in data if e["topic"] == "topic")
        self.assertEqual(topic["parent"], "security/index.html")
        self.assertIn(">", topic["keywords"])
        self.assertIn("A & B", topic["keywords"])

    def test_prerequisite_cycles_are_rejected(self):
        docs = {"index.html": page("Guide"),
                "a.html": page("A", "help", "index.html", prereqs="b"),
                "b.html": page("B", "help", "index.html", prereqs="a")}
        result = self.build(self.fixture("0.90.0", docs))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("prerequisite cycle", result.stderr)

    def test_topic_and_summary_are_canonical(self):
        base = page("Guide")
        cases = [
            (base.replace('<meta name="topic" content="guide">', ""),
             "topic meta is missing; every page needs a stable ID"),
            (base.replace('<meta name="summary" content="Guide summary">', ""),
             "summary meta is missing; every page needs a one-line description"),
            (base.replace('content="guide"', 'content=""'),
             "topic meta is empty; every page needs a stable ID"),
            (base.replace('content="Guide summary"', 'content="  "'),
             "summary meta is empty; every page needs a one-line description"),
            (base.replace('content="Guide summary"',
                          'content="first line&#10;second line"'),
             "summary must be one line"),
            (base.replace('content="Guide summary"', 'content="' + "x" * 161 + '"'),
             "summary is 161 characters; maximum is 160"),
        ]
        for text, message in cases:
            with self.subTest(message=message):
                root = self.fixture("0.90.0", {"index.html": base})
                valid = self.build(root)
                self.assertEqual(valid.returncode, 0, valid.stderr)
                (root / "knowledge/index.html").write_text(text)
                before = self.outputs(root)
                result = self.build(root)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn(message, result.stderr)
                self.assertEqual(before, self.outputs(root))

    def test_duplicate_topic_ids_are_rejected(self):
        a = page("A", "help", "index.html", [("index.html", "Guide")])
        a = a.replace('content="a"', 'content="guide"')
        docs = {"index.html": page("Guide"), "a.html": a}
        result = self.build(self.fixture("0.90.0", docs))
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertIn("duplicate topic 'guide'", result.stderr)

    def test_learning_sidecars_and_truthful_page_path_gaps(self):
        text = page("Guide").replace("</head>", '<meta name="kind" content="tutorial"><meta name="skills" content="shell.novice"></head>')
        root = self.fixture("0.39.0", {"index.html": text})
        self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        entry = self.index(root)["index.html"]
        self.assertEqual(entry["kind"], "tutorial")
        self.assertEqual(entry["skills"], ["shell.novice"])
        self.assertEqual(len((root / "knowledge/index.tsv").read_text().splitlines()[0].split("\t")), 9)
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
                 ('skills', 'missing.novice', 'unknown skill ID'),
                 ('skills', 'shell.novice, shell.novice', 'duplicate skill IDs')]
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
        text = (page('Guide')
                .replace('content="novice"', 'content="learner"')
                .replace('</body>', '<section data-level="learner"><h2>Exercise</h2><p>Try something</p></section></body>'))
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
        text = (page('Pipelines', 'use', 'index.html')
                .replace('content="novice"', 'content="learner"')
                .replace('</body>', '<section data-level="learner"><h2>Task</h2><p>Partial lesson</p></section></body>'))
        root = self.fixture('0.40.0', {'index.html': page('Guide'), 'shell/pipelines.html': text})
        result = self.build(root)
        self.assertEqual(result.returncode, 1)
        self.assertIn('shell/pipelines.html: learner anatomy:', result.stderr)
        self.assertIn('quiz', result.stderr)

    def test_scripted_sandbox_output_counts_as_a_worked_example(self):
        lesson = (
            '<section data-level="learner"><h2>Commands</h2>'
            '<p>Goal: print a greeting.</p><h3>Hello, world!</h3>'
            '<tam-sandbox><tam-cmd match="echo hello">{output}</tam-cmd></tam-sandbox>'
            '<h3>Try it</h3><h3>If it goes wrong</h3>'
            '<details class="q"><summary>What prints?</summary><p>hello</p></details>'
            '</section>'
        )
        path = 'shell/pipelines.html'
        base = page('Pipelines', 'use', 'index.html').replace(
            'content="novice"', 'content="learner"')
        root = self.fixture('0.40.0', {'index.html': page('Guide'), path: base})
        for output in ('<out>hello</out>', '<out> </out>'):
            with self.subTest(output=output):
                (root / 'knowledge' / path).write_text(
                    base.replace('</body>', lesson.format(output=output) + '</body>'))
                result = self.build(root)
                if output == '<out>hello</out>':
                    self.assertEqual(result.returncode, 0, result.stderr)
                else:
                    self.assertEqual(result.returncode, 1)
                    self.assertIn('learner anatomy: missing worked example', result.stderr)

    def test_d221_novice_learner_anatomy_errors_only_on_pilot_pages(self):
        """Non-migrated novice/learner gaps warn; D221 migrated pages hard-fail."""
        partial = (
            '<section data-level="novice"><h2>Intro</h2>'
            '<p><strong>Goal:</strong> do a thing.</p></section>'
        )
        def leveled(title):
            return page(title, 'use', 'index.html').replace(
                '<meta name="level" content="novice">',
                '<meta name="level" content="novice">').replace(
                '</body>', partial + '</body>')
        # Non-migrated path only: warning, build succeeds
        root = self.fixture('0.70.0', {
            'index.html': page('Guide'),
            'ecosystem/terminology.html': leveled('Terminology'),
        })
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('Learning warnings:', result.stdout)
        report = json.loads((root / 'knowledge/coverage.json').read_text())
        self.assertTrue(any(w.get('path') == 'ecosystem/terminology.html' for w in report['warnings']))
        # Add a migrated path with the same gap: hard error
        (root / 'knowledge/shell/pipelines.html').parent.mkdir(parents=True, exist_ok=True)
        (root / 'knowledge/shell/pipelines.html').write_text(leveled('Pipelines'))
        result = self.build(root)
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertIn('shell/pipelines.html: novice anatomy: forbidden goal:', result.stderr)
        self.assertNotIn('ecosystem/terminology.html: novice anatomy:', result.stderr)
    def test_path_skills_span_visited_levels_and_page_skills_work(self):
        text = page('Guide').replace('</head>', '<meta name="kind" content="path"><meta name="skills" content="shell.expert"></head>')
        root = self.fixture('0.40.0', {'index.html': text})
        self.register_skills(root)
        self.assertEqual(self.build(root).returncode, 0)
        report = json.loads((root/'knowledge/coverage.json').read_text())
        self.assertEqual(report['cells'][1]['paths'], ['index.html'])
        (root/'knowledge/index.html').write_text(text.replace('content="path"','content="tutorial"'))
        self.assertEqual(self.build(root).returncode, 0)

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

    def test_cli_aware_tokenisation_mirrors_the_reader(self):
        tokens = search_tokens_mod.tokenize
        def shape(text):
            return [(t["raw"], t["stem"], t["kind"]) for t in tokens(text)]

        self.assertEqual(shape("pipelines and redirection"),
                         [("pipelines", "pipelin", "word"),
                          ("redirection", "redirection", "word")])
        self.assertEqual(shape("--pipe -h"),
                         [("--pipe", "--pipe", "flag"), ("-h", "-h", "flag")])
        self.assertEqual(shape("shell.novice 0.71.0"),
                         [("shell.novice", "shell.novice", "id"),
                          ("0.71.0", "0.71.0", "version")])
        self.assertEqual(shape("terminal/shell/pipelines.html"),
                         [("terminal/shell/pipelines.html",
                           "terminal/shell/pipelines.html", "path")])
        self.assertEqual(shape("limine-snapper-sync"),
                         [("limine-snapper-sync", "limine-snapper-sync", "word")])
        self.assertEqual(shape("C++"), [("C++", "c++", "word")])
        self.assertEqual(shape("how do I pipe"), [("pipe", "pipe", "word")])
        self.assertEqual(shape("bullets - pipe."), [("bullets", "bullet", "word"),
                                                     ("pipe", "pipe", "word")])
        self.assertIsNone(search_tokens_mod.single_token("two words"))
        self.assertIsNone(search_tokens_mod.single_token("the"))
        self.assertEqual(search_tokens_mod.single_token("--pipe")["kind"], "flag")

    def test_search_projection_comes_from_sections_and_metadata(self):
        text = page("Tokens", "use", "index.html").replace(
            "</head>",
            '<meta name="keywords" content="kumquat">'
            '<meta name="search-boost" content="quarkboost">'
            '<meta name="search-exclude" content="quarkhide"></head>'
        ).replace(
            "</body>",
            '<h2 id="details">Details</h2>'
            '<p>kumquat quarkboost quarkhide</p></body>'
        )
        root = self.fixture("0.90.0", {"index.html": page("Guide"),
                                       "tokens.html": text})
        self.assertEqual(self.build(root).returncode, 0)

        entry = self.index(root)["tokens.html"]
        self.assertEqual(entry["search"]["boost"], ["quarkboost"])
        self.assertEqual(entry["search"]["exclude"], ["quarkhide"])
        stems = [term["stem"] for term in entry["search"]["terms"]]
        self.assertIn("kumquat", stems)
        self.assertIn("quarkboost", stems)
        self.assertNotIn("quarkhide", stems)
        self.assertEqual({(a["name"], a["source"]) for a in entry["search"]["anchors"]},
                         {("details", "id"), ("Details", "heading"),
                          ("Tokens", "heading")})

        rows = [line.split("\t") for line in
                (root / "knowledge/index.search.tsv").read_text().splitlines()[1:]]
        boost = next(row for row in rows if row[2] == "quarkboost")
        self.assertEqual(boost[4], "boost")
        self.assertGreater(int(boost[3]), search_tokens_mod.MAX_QUERY_TOKENS)
        excluded = next(row for row in rows if row[2] == "quarkhide")
        self.assertEqual(excluded[3:], ["0", "exclude"])
        anchors = (root / "knowledge/index.anchors.tsv").read_text()
        self.assertIn("tokens.html\tdetails\tid\n", anchors)
        self.assertIn("tokens.html\tTokens\theading\n", anchors)

    def test_text_terms_do_not_demote_authored_fields(self):
        text = page("Kumquat", "use", "index.html").replace(
            "</body>", "<p>kumquat kumquat kumquat</p></body>")
        root = self.fixture("0.90.0", {"index.html": page("Guide"),
                                       "kumquat.html": text})
        self.assertEqual(self.build(root).returncode, 0)
        entry = self.index(root)["kumquat.html"]
        term = next(t for t in entry["search"]["terms"] if t["stem"] == "kumquat")
        self.assertEqual((term["source"], term["weight"]), ("topic", 7))

    def test_invalid_search_metadata_is_rejected(self):
        base = page("Tokens", "use", "index.html").replace(
            "</body>", "<p>quarkboost quarkhide quarkboth</p></body>")
        cases = [
            ('<meta name="search-boost" content="two words">',
             "not a single search term"),
            ('<meta name="search-boost" content="absentterm">',
             "does not appear on the page"),
            ('<meta name="search-boost" content="the">',
             "not a single search term"),
            ('<meta name="search-boost" content="quarkboth">'
             '<meta name="search-exclude" content="quarkboth">',
             "both boosted and excluded"),
        ]
        for meta, message in cases:
            with self.subTest(message=message):
                valid = base.replace("</head>", "</head>")
                root = self.fixture("0.90.0", {"index.html": page("Guide"),
                                               "tokens.html": valid})
                self.assertEqual(self.build(root).returncode, 0)
                before = self.outputs(root)
                (root / "knowledge/tokens.html").write_text(
                    base.replace("</head>", meta + "</head>"))
                result = self.build(root)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn(message, result.stderr)
                self.assertEqual(before, self.outputs(root))

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
        out = io.StringIO()
        err = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            try:
                rc = build_index_mod.main(str(root))
            except SystemExit as e:
                rc = e.code if isinstance(e.code, int) else (1 if e.code else 0)
            except Exception:
                import traceback
                traceback.print_exc(file=err)
                rc = 1
        return BuildResult(rc or 0, out.getvalue(), err.getvalue())

    def make_fixture(self):
        root = self.fixture("0.31.0", {
            "index.html": page("Guide"),
            "use/index.html": page("Using", "use", "index.html"),
        })
        shutil.copyfile(MAKEFILE, root / "Makefile")
        (root / "tools").mkdir()
        shutil.copyfile(BUILDER, root / "tools/build-index.py")
        shutil.copyfile(BUILDER.parent / "search_tokens.py", root / "tools/search_tokens.py")
        for name in ("corpus_pipeline.py", "build-tldr-index.py", "glossary_index.py"):
            shutil.copyfile(BUILDER.parent / name, root / "tools" / name)
        # `make corpus` also regenerates component-coverage.json (plan 26 W1).
        shutil.copyfile(
            COVERAGE_TOOL, root / "tools/report-component-coverage.py"
        )
        (root / ".gitignore").write_text("knowledge/.build_id\nbuild/\n__pycache__/\n")
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
                text = page("Guide").replace("</body>", f'<{tag} data-level="novice">Shared</{tag}></body>')
                root = self.fixture("0.37.1", {"index.html": text})
                self.assertEqual(self.build(root).returncode, 0)
                (root / "knowledge/index.html").write_text(text.replace('data-level="novice"', 'data-level="bogus"'))
                result = self.build(root)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("unknown data-level", result.stdout + result.stderr)
                (root / "knowledge/index.html").write_text(text.replace('data-level="novice"', 'data-level="expert"'))
                self.assertEqual(self.build(root).returncode, 0)

    def test_layered_pages_and_ladders(self):
        shared = page("Guide").replace('content="novice"', 'content="novice, learner, expert"')
        body = ('<aside data-level="novice">Hint</aside>'
                '<p data-level="learner">Model</p>'
                '<table data-level="expert"><tr><td>Reference</td></tr></table>'
                '<section class="quiz" data-level="novice"><h3>Quiz</h3></section>')
        root = self.fixture("0.37.1", {"index.html": shared.replace("</body>", body + "</body>")})
        self.assertEqual(self.build(root).returncode, 0)
        ladder = ''.join(f'<section data-level="{level}"><h2>{level}</h2></section>'
                         for level in ("novice", "learner", "expert"))
        target = root / "knowledge/index.html"
        target.write_text(shared.replace("</body>", ladder + body + "</body>"))
        self.assertEqual(self.build(root).returncode, 0)

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
        self.assertEqual(result.stdout.count("indexes current."), 1)
        for target in targets:
            (root / target).unlink()
            result = self.make_ok(root, target)
            self.assertTrue((root / target).is_file())
            self.assertEqual(result.stdout.count("indexes current."), 1)

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
        for name in (
            "test_lint_corpus.py",
            "test_build_index.py",
            "test_component_coverage.py",
            "test_corpus_pipeline.py",
            "test_glossary_index.py",
        ):
            (root / "tests" / name).write_text(
                "from pathlib import Path\nimport unittest\n"
                "class Check(unittest.TestCase):\n"
                "    def test_generated_index_exists(self):\n"
                "        self.assertTrue(Path('knowledge/index.json').is_file())\n")
        shutil.copyfile(BUILDER.parent / "lint-corpus.py", root / "tools/lint-corpus.py")
        (root / "src").mkdir()
        for name in ("render.c", "html_entities.inc", "tam.c", "install.c"):
            shutil.copyfile(BUILDER.parent.parent / "src" / name, root / "src" / name)
        reader = root / "reader"
        reader.write_text('#!/bin/sh\nprintf "Page\\nRev 1 Last updated 2026-11-11 12:00\\n"\n')
        reader.chmod(0o755)
        guide = root / "tests/guide.sh"
        guide.write_text("#!/bin/sh\nset -eu\ntest -f knowledge/index.tsv\necho 'guide checked'\n")
        guide.chmod(0o755)
        (root / "tools/check-repo.py").write_text("print('repo checked')\n")
        result = self.make_ok(root, "-j8", "corpus-and-test", f"TAM_READER={reader}", "SKIP_COVERAGE_CHECK=1")
        self.assertIn("checks passed", result.stdout)
        self.assertIn("repo checked", result.stdout)
        self.assertLess(result.stdout.index("checks passed"), result.stdout.index("repo checked"))
        result = self.make(root, "test-corpus")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("No rule to make target", result.stderr)

        path = root / "knowledge/use/index.html"
        path.write_text(path.read_text().replace('content="index.html"', 'content="missing.html"'))
        result = self.make(root, "corpus-and-test", f"TAM_READER={reader}", "SKIP_COVERAGE_CHECK=1")
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
        self.assertEqual(len(rows[0].split("\t")), 9)
        self.assertEqual(len(rows[1].split("\t")), 9)
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
        self.assertEqual(rows[0].split("\t")[:9], [
            "topic", "path", "title", "summary", "keywords", "track",
            "prereqs", "rev", "updated"])
        data = {item["path"]: item for item in json.loads(
            (root / "knowledge/index.json").read_text())}
        self.assertEqual(data["use/task.html"]["parent"], "use/index.html")
        self.assertEqual(data["use/task.html"]["topic"], "task")
        self.assertEqual(data["use/task.html"]["levels"], ["novice"])
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

    def test_duplicate_topic_is_rejected(self):
        root = self.fixture("0.81.0", {
            "index.html": page("Guide"),
            "use/first.html": page("First", "use", "index.html").replace(
                'content="first"', 'content="displays"'),
            "maintain/second.html": page("Second", "maintain", "index.html").replace(
                'content="second"', 'content="displays"'),
        })
        result = self.build(root)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("duplicate topic 'displays'", result.stderr)

    def test_meta_paths_build_sidecar_and_validate_steps(self):
        path_a = page("Shell Basics", "learn", "index.html").replace(
            'content="shell-basics"', 'content="shell-basics"'
        ).replace(
            "</head>", '<meta name="kind" content="path"></head>'
        )
        path_b = page("Rescue Flow", "learn", "index.html").replace(
            "</head>", '<meta name="kind" content="path"></head>'
        )
        shared = page("Pipelines", "use", "index.html").replace(
            "</head>", '<meta name="paths" content="shell-basics:2, rescue-flow:1"></head>'
        )
        first = page("Prompt", "use", "index.html").replace(
            "</head>", '<meta name="paths" content="shell-basics:1"></head>'
        )
        root = self.fixture("0.81.0", {
            "index.html": page("Guide"),
            "learn/paths/shell-basics.html": path_a,
            "learn/paths/rescue-flow.html": path_b,
            "use/pipelines.html": shared,
            "use/prompt.html": first,
        })
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)
        tsv = (root / "knowledge/index.paths.tsv").read_text()
        lines = tsv.splitlines()
        self.assertEqual(len(lines[0].split("\t")), 11)
        self.assertEqual(lines[0], "record\tpath_id\targ1\targ2\targ3\targ4\t"
                                   "category\torder\toutcome\tprerequisites\ttime")
        self.assertIn("P\tshell-basics\tShell Basics\tlearn/paths/shell-basics.html", tsv)
        old_row = next(line for line in lines if line.startswith("P\tshell-basics\t"))
        self.assertEqual(old_row.split("\t")[6:], ["", "", "", "", ""])
        self.assertIn("S\tshell-basics\t1\tuse/prompt.html\tPrompt", tsv)
        self.assertIn("S\tshell-basics\t2\tuse/pipelines.html\tPipelines", tsv)
        self.assertIn("S\trescue-flow\t1\tuse/pipelines.html\tPipelines", tsv)

        for bad_meta, err in (
            ("missing-path:1", "unknown learning path 'missing-path'"),
            ("shell-basics", "must be '<path-id>:<step>'"),
            ("shell-basics:0", "must be a positive integer"),
            ("shell-basics:1, shell-basics:2", "duplicate path ID 'shell-basics'"),
            ("shell-basics:1", "duplicate step 1 on path 'shell-basics'"),
        ):
            with self.subTest(bad_meta=bad_meta):
                (root / "knowledge/use/pipelines.html").write_text(
                    shared.replace("shell-basics:2, rescue-flow:1", bad_meta)
                )
                bad = self.build(root)
                self.assertEqual(bad.returncode, 1)
                self.assertIn(err, bad.stderr)

    def test_path_presentation_projects_fields_and_sorts_paths(self):
        def path_page(title, category, order, opening):
            return page(title, "learn", "index.html").replace(
                "</head>",
                '<meta name="kind" content="path">'
                f'<meta name="path-category" content="{category}">'
                f'<meta name="path-order" content="{order}"></head>'
            ).replace("</body>", opening + PATH_BODY + "</body>")

        goal_opening = PATH_OPENING.replace(
            "none beyond reading.",
            '<a href="learn/paths/other.html">Other</a>.')
        documents = {
            "index.html": page("Guide"),
            "help/manuals.html": page("Manuals", "help", "index.html"),
            "use/first.html": page("First", "use", "index.html"),
            "use/second.html": page("Second", "use", "index.html"),
            "use/third.html": page("Third", "use", "index.html"),
            "learn/paths/goal.html": path_page("Goal", "connect", 2, goal_opening),
            "learn/paths/other.html": path_page("Other", "start-here", 1, PATH_OPENING),
        }
        root = self.fixture("0.139.0", documents)
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)

        lines = (root / "knowledge/index.paths.tsv").read_text().splitlines()
        rows = [line.split("\t") for line in lines if line.startswith("P\t")]
        self.assertEqual([row[1] for row in rows], ["other", "goal"])
        goal = next(row for row in rows if row[1] == "goal")
        self.assertEqual(goal[6], "connect")
        self.assertEqual(goal[7], "2")
        self.assertEqual(goal[8], "Read three pages in order.")
        self.assertEqual(goal[9], '<a href="tam:path?id=other">Other</a>.')
        self.assertEqual(goal[10],
                         "About 30 minutes, including checkpoints and the capstone.")
        entry = self.index(root)["learn/paths/goal.html"]
        self.assertEqual(entry["path_category"], "connect")
        self.assertEqual(entry["path_order"], "2")

    def test_path_presentation_rejects_bad_category_or_order(self):
        base = self.path_documents()["learn/paths/goal.html"]
        cases = (
            ('<meta name="path-category" content="bad">'
             '<meta name="path-order" content="1">',
             "unknown path category 'bad'"),
            ('<meta name="path-category" content="start-here">'
             '<meta name="path-order" content="0">',
             "path order must be a positive integer"),
            ('<meta name="path-category" content="start-here">'
             '<meta name="path-order" content="two">',
             "path order must be a positive integer"),
        )
        for meta, message in cases:
            with self.subTest(message=message):
                documents = self.path_documents()
                documents["learn/paths/goal.html"] = base.replace(
                    "</head>", meta + "</head>")
                root = self.fixture("0.139.0", documents)
                result = self.build(root)
                self.assertEqual(result.returncode, 1)
                self.assertIn(message, result.stderr)

    def test_path_contract_accepts_a_conforming_page(self):
        root = self.fixture("0.82.0", self.path_documents())
        result = self.build(root)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_path_contract_is_not_retroactive_before_0_82(self):
        body = PATH_BODY.replace("<h2>Stage 2: Second</h2>", "").replace(
            "<h2>Stage 3: Third</h2>", "")
        root = self.fixture("0.81.0", self.path_documents(body))
        self.assertEqual(self.build(root).returncode, 0)

    def test_path_contract_rejects_each_broken_shape(self):
        cases = (
            ("two stages",
             lambda body: body.replace("<h2>Stage 3: Third</h2>", ""),
             "path needs three to six stages"),
            ("link without a lens",
             lambda body: body.replace('href="use/first.html?level=1"',
                                       'href="use/first.html"', 1),
             "needs a ?level=0-3 lens"),
            ("link with a bad lens",
             lambda body: body.replace('use/first.html?level=1',
                                       'use/first.html?level=7', 1),
             "needs a ?level=0-3 lens"),
            ("five steps in one stage",
             lambda body: body.replace(
                 '<li><a href="use/first.html?level=1">First</a></li>',
                 '<li><a href="use/first.html?level=1">First</a></li>' +
                 "".join('<li><a href="use/second.html?level=1">More</a></li>'
                         for _ in range(4)), 1),
             "needs one to four page steps"),
            ("missing step page",
             lambda body: body.replace('use/first.html?level=1',
                                       'use/absent.html?level=1', 1),
             "does not name an existing page"),
            ("stage without a quiz",
             lambda body: body.replace(STAGE_TWO_QUIZ, ""),
             "stage 2 needs a checkpoint quiz"),
            ("quiz without a question",
             lambda body: body.replace("<summary>Which page?</summary>",
                                       "<summary></summary>"),
             "quiz needs a question summary"),
            ("quiz without an answer",
             lambda body: body.replace(STAGE_ONE_QUIZ,
                                       '<details class="q"><summary>Which page?</summary></details>'),
             "quiz needs an answer"),
            ("capstone without a solution",
             lambda body: body.replace(CAPSTONE_SOLUTION, ""),
             "capstone needs exactly one folded solution"),
            ("capstone with two solutions",
             lambda body: body.replace(CAPSTONE_SOLUTION, CAPSTONE_SOLUTION * 2),
             "capstone needs exactly one folded solution"),
            ("missing capstone heading",
             lambda body: body.replace("<h2>Capstone: Put it together</h2>",
                                       "<h2>Practice</h2>"),
             "path needs exactly one capstone heading"),
            ("next paths without a link",
             lambda body: body.replace(
                 '<li><a href="use/first.html?level=1">First again</a></li>',
                 "<li>More later.</li>"),
             "Next paths needs at least one internal link"),
            ("missing next paths heading",
             lambda body: body.replace("<h2>Next paths</h2>",
                                       "<h2>Afterwards</h2>"),
             "path needs one Next paths heading"),
        )
        for label, mutate, message in cases:
            with self.subTest(case=label):
                root = self.fixture("0.82.0", self.path_documents(mutate(PATH_BODY)))
                result = self.build(root)
                self.assertEqual(result.returncode, 1, result.stdout)
                self.assertIn(message, result.stderr)
                self.assertNotIn("Traceback", result.stderr)

    def test_path_rejects_each_broken_opening_field(self):
        cases = (
            ("missing outcome",
             lambda opening: opening.replace(
                 "<p><strong>Outcome:</strong> Read three pages in order.</p>", ""),
             "path opening needs a outcome line"),
            ("empty outcome",
             lambda opening: opening.replace(
                 "Read three pages in order.", ""),
             "path opening needs a outcome line"),
            ("missing prerequisites",
             lambda opening: opening.replace(
                 "<p><strong>Prerequisites:</strong> none beyond reading.</p>", ""),
             "path opening needs a prerequisites line"),
            ("bad prerequisite link",
             lambda opening: opening.replace(
                 "none beyond reading.", '<a href="use/absent.html">Absent</a>'),
             "prerequisite link"),
            ("missing time",
             lambda opening: opening.replace(
                 "<p><strong>Time:</strong> About 30 minutes, including checkpoints "
                 "and the capstone.</p>", ""),
             "path opening needs a time line"),
            ("wrong time",
             lambda opening: opening.replace("About 30 minutes", "About 45 minutes"),
             "time should be about 30 minutes for 3 stages"),
            ("missing stuck note",
             lambda opening: opening.split("<aside", 1)[0],
             "path opening needs an If you're stuck note"),
            ("stuck note without note class",
             lambda opening: opening.replace('class="note"', 'class="tip"'),
             "path opening needs an If you're stuck note"),
            ("stuck note without its label",
             lambda opening: opening.replace("<strong>If you're stuck:</strong>",
                                              "<strong>Help:</strong>"),
             "path opening needs an If you're stuck note"),
            ("stuck note without manuals command",
             lambda opening: opening.replace(
                 " with <code>tam man &lt;command&gt;</code>,", ""),
             "failure branch needs manuals"),
            ("stuck note without manuals page",
             lambda opening: opening.replace(
                 '<a href="help/manuals.html">program manuals</a>', "the manuals"),
             "failure branch needs manuals page"),
            ("stuck note without built-in help",
             lambda opening: opening.replace("<code>tam help</code>, or", "or"),
             "failure branch needs built-in help"),
            ("stuck note without search",
             lambda opening: opening.replace(
                 "\n<code>tam search -- &lt;term&gt;</code>", ""),
             "failure branch needs search"),
        )
        for label, mutate, message in cases:
            with self.subTest(case=label):
                root = self.fixture("0.82.0", self.path_documents(
                    opening=mutate(PATH_OPENING)))
                result = self.build(root)
                self.assertEqual(result.returncode, 1, result.stdout)
                self.assertIn(message, result.stderr)
                self.assertNotIn("Traceback", result.stderr)

    def test_path_rejects_a_checkpoint_without_expected_state(self):
        body = PATH_BODY.replace(
            "<p><strong>Expected sandbox state:</strong> No sandbox in this stage; "
            "no simulated files or output are expected.</p>", "")
        root = self.fixture("0.82.0", self.path_documents(body))
        result = self.build(root)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("checkpoint 1 needs an expected sandbox state", result.stderr)
        self.assertNotIn("Traceback", result.stderr)

    def test_path_steps_must_be_contiguous_from_one(self):
        documents = self.path_documents()
        for rel, step in (("use/second.html", 2), ("use/third.html", 4)):
            documents[rel] = documents[rel].replace(
                "</head>", f'<meta name="paths" content="goal:{step}"></head>')
        root = self.fixture("0.82.0", documents)
        result = self.build(root)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("steps [2, 4] are not contiguous from 1", result.stderr)

    def path_documents(self, body=None, opening=None):
        body = PATH_BODY if body is None else body
        opening = PATH_OPENING if opening is None else opening
        return {
            "index.html": page("Guide"),
            "help/manuals.html": page("Manuals", "help", "index.html"),
            "use/first.html": page("First", "use", "index.html"),
            "use/second.html": page("Second", "use", "index.html"),
            "use/third.html": page("Third", "use", "index.html"),
            "learn/paths/goal.html": page("Goal", "learn", "index.html").replace(
                "</head>", '<meta name="kind" content="path"></head>'
            ).replace("</body>", opening + body + "</body>"),
        }


PATH_OPENING = """
<p><strong>Outcome:</strong> Read three pages in order.</p>
<p><strong>Prerequisites:</strong> none beyond reading.</p>
<p><strong>Time:</strong> About 30 minutes, including checkpoints and the capstone.</p>
<aside class="note"><p><strong>If you're stuck:</strong> read the
<a href="help/manuals.html">program manuals</a> with <code>tam man &lt;command&gt;</code>,
list built-in help with <code>tam help</code>, or search this guide with
<code>tam search -- &lt;term&gt;</code>.</p></aside>
"""

PATH_BODY = """
<h2>Stage 1: First</h2>
<ol><li><a href="use/first.html?level=1">First</a></li></ol>
<section class="quiz"><h3>Checkpoint</h3>
<details class="q"><summary>Which page?</summary><p>The first page.</p><p><strong>Expected sandbox state:</strong> No sandbox in this stage; no simulated files or output are expected.</p></details></section>
<h2>Stage 2: Second</h2>
<ol><li><a href="use/second.html?level=1">Second</a></li></ol>
<section class="quiz"><h3>Checkpoint</h3>
<details class="q"><summary>Which page now?</summary><p>The second page.</p><p><strong>Expected sandbox state:</strong> No sandbox in this stage; no simulated files or output are expected.</p></details></section>
<h2>Stage 3: Third</h2>
<ol><li><a href="use/third.html?level=1">Third</a></li></ol>
<section class="quiz"><h3>Checkpoint</h3>
<details class="q"><summary>Last page?</summary><p>The third page.</p><p><strong>Expected sandbox state:</strong> No sandbox in this stage; no simulated files or output are expected.</p></details></section>
<h2>Capstone: Put it together</h2>
<p>Do the whole task in a scratch folder.</p>
<details class="solution"><p>Worked answer.</p></details>
<h2>Next paths</h2>
<ul><li><a href="use/first.html?level=1">First again</a></li></ul>
"""

STAGE_ONE_QUIZ = ('<details class="q"><summary>Which page?</summary>'
                  '<p>The first page.</p><p><strong>Expected sandbox state:</strong> '
                  'No sandbox in this stage; no simulated files or output are '
                  'expected.</p></details>')

STAGE_TWO_QUIZ = """
<section class="quiz"><h3>Checkpoint</h3>
<details class="q"><summary>Which page now?</summary><p>The second page.</p><p><strong>Expected sandbox state:</strong> No sandbox in this stage; no simulated files or output are expected.</p></details></section>"""

CAPSTONE_SOLUTION = '<details class="solution"><p>Worked answer.</p></details>'


if __name__ == "__main__":
    unittest.main()
