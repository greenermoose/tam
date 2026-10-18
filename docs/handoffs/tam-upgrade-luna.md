# Luna implementation handoff: tam upgrade and switch

**Approved:** 2026-10-18. The developer accepted the complete
[work plan](../plans/upgrade.md); the verbatim
[approval record](../ai/2026-10-18-tam-update-check-plan.md#approval-and-handoff)
records the handoff request. This file is the implementation handoff; the work
plan owns requirements and acceptance. Automated implementation evidence is
recorded there. The isolated real source build and developer-directed first
activation remain outstanding.

## Start here

Work in the existing `tam` clone on develop. Read `AGENTS.md`,
[the roadmap](../../ROADMAP.md), [the work plan](../plans/upgrade.md), and the
current sections of [the development contract](../plans/development.md).
Run `git status --short` and inspect the current head before selecting scopes.
Use your own session identity and claim only paths being edited. Do not create
a worktree, change branches or adopt this planning session's claims.

Planning baseline: `b131bab`, command **0.30.0**, corpus **0.31.0**.
Implementation versions are command **0.31.0**, corpus **0.32.0**. Current
acceptance and test evidence are in the plan; source-build and activation limits
remain explicit below.

## Implement in order

1. **TCK6 (version discovery and comparison):** independent command/corpus
   comparison, stage resolution, paired release tags, running identity,
   bounded HTTPS and strict metadata parsing. Start with deterministic
   comparator and metadata fixtures, then the checker integration.
2. **TCK7 (source build and transactional activation):** exact libtam pin,
   source-build manifest and audited older-source compatibility, isolated
   generations, owned links, receipts, atomic activation, locking and offline
   recovery. Keep a usable updater/recovery path after switching to an old tam
   that has no upgrade/switch subcommands.
3. **TCK8 (CLI, documentation and delivery handoff):** finish both command
   surfaces and help/completions/manual/corpus documentation, independent
   version bumps, complete acceptance evidence and bootstrap instructions.

Follow each item's completion evidence in the plan. Do not close the feature
after a checker alone; source installation and stage switching are required.
Wire minimal CLI/build support while developing TCK6/TCK7 as needed; TCK8 owns
the final integrated surface and documentation. Update item status and evidence
as each outcome lands, keeping plan/index/roadmap/contract consistent.

Suggested first claim, adjusted to the actual files you will edit:

```sh
tam-work begin --scope src/upgrade.c --scope src/upgrade.h \
  --scope tests/upgrade.sh --scope tests/fixtures/upgrade \
  --scope docs/plans/upgrade.md --note 'TCK6: version discovery and comparison'
```

Extend the claim before touching `Makefile`, `src/tam.c`, version assertions,
shared entry points, parser modules or other paths. Whole-tree builds/tests
need a short exclusive claim under the repository's concurrency rules.

## Accepted decisions to preserve

- CLI: bare `tam upgrade` checks all stages; `upgrade --apply` updates the
  tracked stage; `switch STAGE` explicitly changes stage/version, including
  confirmed downgrades; `switch --rollback` recovers offline.
- Development maps to develop, Test to test, Run to main, Release to a tagged
  noteworthy Run commit. These select versions locally and never promote code.
- Source builds install under `~/.local/dev` with private per-generation
  library/corpus. Never replace unrelated shared development libraries or
  overwrite unowned files. No automatic OS dependency installation.
- Command and corpus versions follow code changes independently; new code
  builds for the developer need bumps. Rebuilding existing numbered source
  preserves the source's versions. Release tags identify both, such as
  `tam-0.31.0-corpus-0.32.1`; no release is required for every Run version.
- Keep ordinary browsing/help/version offline. Live checks are explicit,
  bounded and truthful about unavailable/unknown stages and partial failure.

## Validation and return to the developer

Use the plan's acceptance matrix and isolated fake-HOME fixtures; normal
automated tests must not depend on live GitHub. Cover interrupted/failed builds,
endpoint conflicts, locking, older-stage installs, exact release pairs, pins,
corpus-only updates and rollback. Exercise real source builds in an isolated
prefix and verify them away from the build tree. This planning session ran
documentation checks only; it provides no runtime acceptance evidence.

The first version containing these commands needs an explicit bootstrap;
an installed old tam cannot call commands it does not have. Prepare that
procedure with dependencies, absolute target/version verification and rollback.
Obtain the developer's instruction before activating the real development
prefix. Test/Run/Release promotions and packaged deployment still need their
own exact-candidate instructions; plan acceptance grants none of those actions.

### Bootstrap the first version containing `upgrade` and `switch`

An older installed `tam` cannot invoke these commands. Once the implementation
commit is available, use the normal component-development procedure to build
that exact commit under `~/.local/dev`; do not try to bootstrap through the old
command. In a terminal, verify the candidate and its source-build pin first:

```sh
git -C ~/Code/tamlinux/tam rev-parse HEAD
git -C ~/Code/tamlinux/tam show HEAD:src/version.h
git -C ~/Code/tamlinux/tam show HEAD:knowledge/version
git -C ~/Code/tamlinux/tam show HEAD:source-build.json
```

Use temporary, detached source snapshots at the exact reviewed tam commit and
the `libtam_revision` in its manifest. Do not switch either developer checkout.
The build needs `cc`, `make`, `pkg-config`, Python 3, Git, curl, zlib, ncursesw,
and D-Bus development files. Set `TAM_REV` to the verified full Develop commit
and `LIBTAM_REV` to the full manifest pin, then run:

```sh
bootstrap=$(mktemp -d)
git -c core.hooksPath=/dev/null clone --quiet --no-checkout --filter=blob:none --no-recurse-submodules https://github.com/greenermoose/libtam.git "$bootstrap/libtam"
git -C "$bootstrap/libtam" -c core.hooksPath=/dev/null checkout --quiet --detach "$LIBTAM_REV"
git -c core.hooksPath=/dev/null clone --quiet --no-checkout --filter=blob:none --no-recurse-submodules https://github.com/greenermoose/tam.git "$bootstrap/tam"
git -C "$bootstrap/tam" -c core.hooksPath=/dev/null checkout --quiet --detach "$TAM_REV"
test "$(git -C "$bootstrap/libtam" rev-parse HEAD)" = "$LIBTAM_REV"
test "$(git -C "$bootstrap/tam" rev-parse HEAD)" = "$TAM_REV"
make -C "$bootstrap/libtam" test
make -C "$bootstrap/libtam" install PREFIX="$HOME/.local/dev/opt/tam/bootstrap-libtam"
make -C "$bootstrap/tam" test PREFIX="$HOME/.local/dev" LIBTAM_PREFIX="$HOME/.local/dev/opt/tam/bootstrap-libtam" TAM_TEST_SKIP_OLLAMA=1
make -C "$bootstrap/tam" install PREFIX="$HOME/.local/dev" LIBTAM_PREFIX="$HOME/.local/dev/opt/tam/bootstrap-libtam"
hash -r
type -a tam
readlink -f "$HOME/.local/dev/bin/tam"
tam --version
ldd "$HOME/.local/dev/bin/tam" | rg "$HOME/.local/dev/opt/tam/bootstrap-libtam/lib/libtam.so"
mkdir -p "$HOME/.local/dev/opt/tam/updater"
install -m 755 "$HOME/.local/dev/bin/tam-upgrade" "$HOME/.local/dev/opt/tam/updater/tam-upgrade"
make -C "$bootstrap/tam" uninstall PREFIX="$HOME/.local/dev" LIBTAM_PREFIX="$HOME/.local/dev/opt/tam/bootstrap-libtam"
hash -r
"$HOME/.local/dev/opt/tam/updater/tam-upgrade" switch development
type -a tam
readlink -f "$HOME/.local/dev/bin/tam"
tam --version
```

Confirm the resolved executable is under `~/.local/dev`, its command and
corpus versions match the candidate files, and `ldd` resolves libtam from the
private bootstrap path. The recovery helper is copied to its retained location
before the ordinary component install is removed; this leaves the packaged
command as fallback if the first managed build fails. Keep the detached source
snapshots until the managed command and rollback are verified, then remove the
temporary bootstrap directory. Remove the private bootstrap libtam only if no
other tool uses it. This does not install OS packages or deploy a Tamlinux
assembly; subsequent source switches use isolated generations and receipts.

The implementation has automated fake-HOME coverage but has not run a real
upstream build. The earlier HTTP 404 came from requesting the repository root
with a trailing slash (`627c126` fixes it); the repository is public and the
push and `gh` failures were the agent sandbox lacking Fred's SSH agent and
keyring, not revoked access. Perform the plan's preview-confirmed isolated
build next. Request Fred's instruction before
activating the real development prefix.
Package Test/Run/Release promotions and deployments are outside this handoff.

Commit only claimed paths, with current runtime provenance. Record the
implementation commit, component versions, tests, bootstrap procedure and
remaining human acceptance in the plan/session record. Release claims at
completion.
