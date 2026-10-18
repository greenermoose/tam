import importlib.machinery
import fcntl
import json
import os
import subprocess
import pathlib
import tempfile
import unittest
from types import SimpleNamespace
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
module = importlib.machinery.SourceFileLoader("tam_upgrade", str(ROOT / "bin" / "tam-upgrade")).load_module()


class UpgradeMetadataTests(unittest.TestCase):
    def test_semver_fixture_matrix(self):
        cases = json.loads((ROOT / "tests/fixtures/upgrade/semver.json").read_text())
        for case in cases:
            with self.subTest(case=case):
                self.assertEqual(module.compare_semver(case["older"], case["newer"]), case["order"])

    def test_invalid_versions_are_rejected(self):
        for version in ("01.0.0", "1.0", "1.0.0-01", "9223372036854775808.0.0"):
            with self.subTest(version=version), self.assertRaises(ValueError):
                module.semver(version)

    def test_version_summary_accepts_display_build_labels(self):
        self.assertEqual(module.version_summary(
            b"tam command 0.29.3 (dev 518cad5)\nknowledge corpus 0.30.0 (dev 9de0735-dirty)\n"),
            ("0.29.3", "0.30.0"))

    def test_duplicate_and_overdeep_json_are_rejected(self):
        with self.assertRaises(ValueError):
            module.parse_json('{"status":"ok","status":"bad"}')
        nested = "0"
        for _ in range(34):
            nested = "[" + nested + "]"
        with self.assertRaises(ValueError):
            module.parse_json(nested)

    def test_comparison_keeps_unknown_and_running_ahead_distinct(self):
        self.assertEqual(module.comparison(None, "1.0.0"), "unknown")
        self.assertEqual(module.comparison("2.0.0", "1.9.9"), "running_ahead")
        self.assertEqual(module.comparison("1.0.0", "1.0.0"), "same_version")

    def test_legacy_build_pin_is_exact_and_audited(self):
        pin = module.source_build_manifest("abc1380dea7033d5ebab5336d66ec216b99e1e66", 0)
        self.assertEqual(pin["libtam_revision"], "c081708b5ed2c8ade4ecdf171cebc110c2cb316f")
        self.assertIn("delivery", pin["evidence"])
        self.assertEqual(len(module.LEGACY_BUILDS), 5)
        self.assertTrue(all(p["libtam_revision"] == pin["libtam_revision"]
                            for p in module.LEGACY_BUILDS.values()))

    def test_release_pair_discovery_and_conflicting_alias(self):
        sha1, sha2, main = "1" * 40, "2" * 40, "3" * 40
        refs = [{"ref":"refs/tags/tam-0.30.0-corpus-0.31.0", "object":{"type":"commit", "sha":sha1}},
                {"ref":"refs/tags/tam-0.31.0-corpus-0.32.0", "object":{"type":"commit", "sha":sha2}}]
        def api(path, deadline, allow_404=False):
            if path == "/git/ref/heads/main": return {"object":{"sha":main}}
            if path.startswith("/git/matching-refs/"): return refs if "page=1" in path else []
            if path.startswith("/compare/"): return {"status":"ahead"}
            raise AssertionError(path)
        def versions(sha, deadline):
            return {sha1:("0.30.0","0.31.0"), sha2:("0.31.0","0.32.0")}[sha]
        with mock.patch.object(module, "api_json", side_effect=api), mock.patch.object(module, "versions_at", side_effect=versions):
            releases = module.release_inventory(100)
        self.assertEqual([r["command"] for r in releases], ["0.30.0", "0.31.0"])
        self.assertEqual(releases[-1]["revision"], sha2)
        sha3 = "4" * 40
        refs.append({"ref":"refs/tags/v0.30.0", "object":{"type":"commit", "sha":sha3}})
        def versions_with_alias(sha, deadline):
            return {sha1:("0.30.0","0.31.0"), sha2:("0.31.0","0.32.0"), sha3:("0.30.0","0.31.0")}[sha]
        with mock.patch.object(module, "api_json", side_effect=api), mock.patch.object(module, "versions_at", side_effect=versions_with_alias):
            with self.assertRaises(module.CheckError) as caught: module.release_inventory(100)
        self.assertEqual(caught.exception.code, "release_conflict")

    def test_help_and_invalid_mutation_arguments_are_offline(self):
        with tempfile.TemporaryDirectory() as home:
            env = {"HOME":home, "PATH":os.environ.get("PATH", "")}
            help_run = subprocess.run([str(ROOT / "bin/tam-upgrade"), "upgrade", "--help"],
                                      capture_output=True, env=env, check=False)
            self.assertEqual(help_run.returncode, 0)
            self.assertIn(b"bounded HTTPS", help_run.stdout)
            invalid = subprocess.run([str(ROOT / "bin/tam-upgrade"), "switch", "--version", "1.2.3"],
                                     capture_output=True, env=env, check=False)
            self.assertEqual(invalid.returncode, 2)
            self.assertFalse((pathlib.Path(home) / ".local").exists())

    def test_bounded_http_reader_enforces_headers_body_and_deadline(self):
        def bounded(proc, deadline):
            try: return module._read_bounded(proc, deadline)
            finally:
                if proc.poll() is None:
                    proc.kill(); proc.wait()
                proc.stdout.close()
        good = subprocess.Popen(["python3", "-c", "import sys; sys.stdout.buffer.write(b'HTTP/1.1 200 OK\\r\\n\\r\\nbody')"],
                                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        self.assertEqual(bounded(good, module.time.monotonic() + 2)[:2], (200, b"body"))
        headers = subprocess.Popen(["python3", "-c", "import sys; sys.stdout.buffer.write(b'x'*40000)"],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        with self.assertRaises(module.CheckError) as caught:
            bounded(headers, module.time.monotonic() + 2)
        self.assertEqual(caught.exception.code, "headers_too_large")
        body = subprocess.Popen(["python3", "-c", "import sys; sys.stdout.buffer.write(b'HTTP/1.1 200 OK\\r\\n\\r\\n'+b'x'*(1024*1024+1))"],
                               stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        with self.assertRaises(module.CheckError) as caught:
            bounded(body, module.time.monotonic() + 3)
        self.assertEqual(caught.exception.code, "too_large")
        slow = subprocess.Popen(["python3", "-c", "import time; time.sleep(3)"],
                                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        with self.assertRaises(module.CheckError) as caught:
            bounded(slow, module.time.monotonic() + 0.1)
        self.assertEqual(caught.exception.code, "timeout")

    def test_repository_root_url_has_no_trailing_slash(self):
        # GitHub returns 404 for /repos/<owner>/<repo>/, so the root must be requested bare.
        urls = []
        def popen(argv, **kwargs):
            urls.append(argv[-1])
            raise FileNotFoundError
        with mock.patch.object(module.subprocess, "Popen", popen):
            with self.assertRaises(module.CheckError):
                module.request("")
        self.assertEqual(urls, [module.API])
        for path in ("/", "//x", "x", "/x\n"):
            with self.assertRaises(module.CheckError) as caught:
                module.request(path)
            self.assertEqual(caught.exception.code, "invalid_request")

    def test_stage_check_preserves_partial_results_and_failures(self):
        revisions = {"develop": "1" * 40, "test": "2" * 40, "main": "3" * 40}
        def api(path, deadline, allow_404=False):
            if path == "": return {"full_name": "greenermoose/tam"}
            branch = path.rsplit("/", 1)[-1]
            if branch == "main": raise module.CheckError("rate_limited", "fixture limit")
            return {"object": {"sha": revisions[branch]}}
        def versions(sha, deadline):
            return {"1" * 40: ("0.31.0", "0.32.0"), "2" * 40: ("0.29.3", "0.31.0")}[sha]
        env = {"TAM_UPGRADE_RUNNING_COMMAND":"0.30.0", "TAM_UPGRADE_RUNNING_CORPUS":"0.31.0",
               "TAM_UPGRADE_RUNNING_REVISION":"4" * 40, "TAM_UPGRADE_RUNNING_DIRTY":"false"}
        with mock.patch.dict(os.environ, env), mock.patch.object(module, "api_json", side_effect=api), \
             mock.patch.object(module, "versions_at", side_effect=versions):
            result, status = module.check_stages(module.STAGES)
        self.assertEqual(status, 1)
        self.assertEqual([x["status"] for x in result["stages"]], ["available", "available", "error"])
        self.assertTrue(result["stages"][0]["update_available"])
        self.assertEqual(result["stages"][0]["corpus_comparison"], "newer")
        self.assertEqual(result["stages"][2]["error"], "rate_limited")

    def _activate_fixture(self, home, sha, fail_tam_test=False, preexisting=False, prompts=None):
        item = {"stage":"development", "command":"0.31.0", "corpus":"0.32.0",
                "revision":sha, "build":{"libtam_revision":"c081708b5ed2c8ade4ecdf171cebc110c2cb316f",
                "libtam_version":"0.2.0"}}
        def fake_run(argv, env, timeout, output_limit, log_path, cwd=None):
            if argv[0] == "git" and "init" in argv:
                clone = pathlib.Path(argv[-1]); clone.mkdir()
                if clone.name == "libtam":
                    (clone / "include").mkdir()
                    (clone / "include/tam.h").write_text('#define TAM_VERSION_STRING "0.2.0"\n')
            if argv[:2] == ["make", "test"] and cwd and cwd.name == "tam" and fail_tam_test:
                raise module.CheckError("build_failed", "injected test failure")
            if argv[:2] == ["make", "install"] and cwd and cwd.name == "tam":
                prefix_arg = next(x for x in argv if x.startswith("PREFIX="))
                gen = pathlib.Path(prefix_arg.split("=", 1)[1])
                (gen / "bin").mkdir(parents=True, exist_ok=True)
                for name in ("tam", "tam-man", "tam-info", "tam-upgrade"):
                    (gen / "bin" / name).write_text("#!/bin/sh\nexit 0\n")
            return None
        package_bin = home / "packaged/bin"
        package_bin.mkdir(parents=True, exist_ok=True)
        (package_bin / "tam").write_text("#!/bin/sh\nexit 0\n")
        (package_bin / "tam").chmod(0o755)
        if preexisting:
            old = home / ".local/dev/bin/tam"
            old.parent.mkdir(parents=True, exist_ok=True)
            old.write_text("unowned endpoint\n")
        def fake_subprocess_run(argv, **kwargs):
            if argv[0] == "git" and argv[-2:] == ["rev-parse", "HEAD"]:
                output = sha if pathlib.Path(argv[2]).name == "tam" else "c081708b5ed2c8ade4ecdf171cebc110c2cb316f"
                return SimpleNamespace(returncode=0, stdout=output + "\n")
            if argv[-1] == "--version":
                if str(argv[0]) == str(package_bin / "tam"):
                    return SimpleNamespace(returncode=0, stdout=b"tam command 0.30.0\n")
                output = "tam command 0.31.0\nknowledge corpus 0.32.0\nlibtam 0.2.0\n"
                return SimpleNamespace(returncode=0, stdout=output if kwargs.get("text") else output.encode())
            return SimpleNamespace(returncode=0, stdout=b"")
        def accept_preview(prompt):
            if prompts is not None:
                prompts.append(prompt)
            return True
        with mock.patch.dict(os.environ, {"HOME":str(home), "PATH":os.environ.get("PATH", ""),
                                          "TAM_UPGRADE_RUNNING_COMMAND":"0.30.0",
                                          "TAM_UPGRADE_RUNNING_CORPUS":"0.31.0",
                                          "TAM_UPGRADE_RUNNING_REVISION":"unknown"}), \
             mock.patch.object(module.sys.stdin, "isatty", return_value=True), \
             mock.patch.object(module, "confirm", side_effect=accept_preview), \
             mock.patch.object(module, "preflight"), \
             mock.patch.object(module, "run_bounded", side_effect=fake_run), \
             mock.patch.dict(os.environ, {"HOME":str(home), "PATH":str(package_bin) + os.pathsep + os.environ.get("PATH", "")}), \
             mock.patch.object(module.subprocess, "run", side_effect=fake_subprocess_run):
            return module.activate(item)

    def test_fake_home_activation_and_rollback_keep_generation_atomic(self):
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            self.assertEqual(self._activate_fixture(home, "1" * 40), 0)
            current = home / ".local/dev/opt/tam/current"
            first = os.readlink(current)
            self.assertTrue(list((home / ".local/state/tam-upgrade/receipts").glob("*.json")))
            self.assertEqual(self._activate_fixture(home, "2" * 40), 0)
            self.assertNotEqual(os.readlink(current), first)
            previous_output = b"tam command 0.31.0\nknowledge corpus 0.32.0\nlibtam 0.2.0\n"
            with mock.patch.dict(os.environ, {"HOME":str(home)}), \
                 mock.patch.object(module.subprocess, "run", return_value=SimpleNamespace(returncode=0, stdout=previous_output)):
                self.assertEqual(module.rollback(), 0)
            self.assertEqual(os.readlink(current), first)

    def test_build_preview_shows_current_target_revisions_paths_and_rollback(self):
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            prompts = []
            self.assertEqual(self._activate_fixture(home, "1" * 40, prompts=prompts), 0)
            self.assertEqual(len(prompts), 1)
            preview = prompts[0]
            for expected in (
                "Current: tam 0.30.0 / corpus 0.31.0",
                "Target: tam 0.31.0 / corpus 0.32.0 (development)",
                "tam " + "1" * 40,
                "libtam 0.2.0 at c081708b5ed2c8ade4ecdf171cebc110c2cb316f",
                f"Destination: {home}/.local/dev",
                f"Rollback target: {home}/packaged/bin/tam (tam 0.30.0 / corpus unknown)",
                "downloads pinned sources and runs their Makefiles",
            ):
                self.assertIn(expected, preview)

    def test_failed_build_keeps_previous_current_pointer(self):
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            self._activate_fixture(home, "1" * 40)
            current = home / ".local/dev/opt/tam/current"
            before = os.readlink(current)
            with self.assertRaises(module.CheckError):
                self._activate_fixture(home, "2" * 40, fail_tam_test=True)
            self.assertEqual(os.readlink(current), before)

    def test_unowned_endpoint_and_active_lock_are_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaises(module.CheckError) as caught:
                self._activate_fixture(pathlib.Path(tmp), "1" * 40, preexisting=True)
            self.assertEqual(caught.exception.code, "ownership")
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            outside = home / "outside"
            outside.mkdir()
            (home / ".local").symlink_to(outside, target_is_directory=True)
            with self.assertRaises(module.CheckError) as caught:
                self._activate_fixture(home, "1" * 40)
            self.assertEqual(caught.exception.code, "ownership")
            self.assertFalse((outside / "dev").exists())
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            self._activate_fixture(home, "1" * 40)
            current = home / ".local/dev/opt/tam/current"
            before = os.readlink(current)
            updater = home / ".local/dev/opt/tam/updater/tam-upgrade"
            updater.unlink()
            outside = home / "outside-updater"
            outside.write_text("user file")
            updater.symlink_to(outside)
            with self.assertRaises(module.CheckError) as caught:
                self._activate_fixture(home, "2" * 40)
            self.assertEqual(caught.exception.code, "ownership")
            self.assertEqual(outside.read_text(), "user file")
            self.assertEqual(os.readlink(current), before)
        with tempfile.TemporaryDirectory() as tmp:
            home = pathlib.Path(tmp)
            lock = home / ".local/state/tam-upgrade/install.lock"
            lock.parent.mkdir(parents=True)
            lock.write_bytes(module.LOCK_MARKER)
            lock_fd = os.open(lock, os.O_RDWR)
            fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
            try:
                with self.assertRaises(module.CheckError) as caught:
                    self._activate_fixture(home, "1" * 40)
                self.assertEqual(caught.exception.code, "locked")
            finally:
                os.close(lock_fd)


if __name__ == "__main__":
    unittest.main()
