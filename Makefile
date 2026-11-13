# tam - the Tamlinux command
#
# Builds the command against an installed libtam. The library is a separate
# dependency and is never built or modified by this Makefile.
#
# libtam 0.6.0 or later is linked statically. It is found with
# `pkg-config libtam`, wherever it is installed; add
# <prefix>/lib/pkgconfig to PKG_CONFIG_PATH for a prefix outside pkg-config's
# default path. Set LIBTAM_PREFIX to name the install prefix instead, or
# LIBTAM_CFLAGS and LIBTAM_LIBS directly for a custom layout.
#
# Two workstream lanes build independently:
#   make command   the C binary, linking libtam
#   make corpus    the knowledge corpus index
# `make` (or `make all`) builds both. See docs/plans/development.md.

CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra -Wpedantic -std=c11

# When Nix is detected (e.g. Tamlinux with Home Manager, where ~/.local/bin is
# store-linked and /usr/local is retired), default development installs to the
# user development prefix $(HOME)/.local/dev rather than root-owned /usr/local.
ifndef PREFIX
  ifneq ($(or $(NIX_PROFILES),$(wildcard $(HOME)/.nix-profile)),)
    PREFIX := $(HOME)/.local/dev
  else
    PREFIX := /usr/local
  endif
endif

BINDIR   ?= $(PREFIX)/bin
DATADIR  ?= $(PREFIX)/share/tamlinux/knowledge
MANDIR   ?= $(PREFIX)/share/man
BASHCOMPDIR ?= $(PREFIX)/share/bash-completion/completions

PKG_CONFIG ?= pkg-config
ifdef LIBTAM_PREFIX
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
LIBTAM_ERROR  = $(if $(shell grep -s -l tam_bundle_guard $(LIBTAM_PREFIX)/include/tam.h),,LIBTAM_PREFIX=$(LIBTAM_PREFIX) has no libtam 0.6.0 or later include/tam.h)
else ifeq ($(origin LIBTAM_LIBS),undefined)
LIBTAM_FOUND  = $(shell $(PKG_CONFIG) --atleast-version=0.6.0 libtam 2>/dev/null && echo yes)
LIBTAM_CFLAGS = $(if $(LIBTAM_FOUND),$(shell $(PKG_CONFIG) --cflags libtam))
LIBTAM_LIBS   = $(if $(LIBTAM_FOUND),$(shell $(PKG_CONFIG) --libs libtam))
LIBTAM_ERROR  = $(if $(LIBTAM_FOUND),,$(PKG_CONFIG) cannot find libtam 0.6.0 or later)
endif

LDFLAGS  ?=
LDLIBS   ?= -lz
TERMINFO_CFLAGS ?= $(shell pkg-config --cflags ncursesw 2>/dev/null)
TERMINFO_LIBS   ?= $(shell pkg-config --libs ncursesw 2>/dev/null || echo -ltinfo)

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c src/identity.c src/corpus.c src/views.c src/address.c src/html.c src/html_scan.c src/render.c src/display.c src/pager.c src/pager_draw.c src/pager_input.c src/pager_links.c src/pager_nav.c src/pager_search.c src/pager_screen.c src/external.c src/terminal.c src/install.c src/man.c src/index.c src/depth.c src/fold.c src/ask.c src/info.c src/discovery.c src/tldr.c src/upgrade.c
INDEX := knowledge/index.tsv knowledge/index.json knowledge/index.parents.tsv knowledge/index.kinds.tsv knowledge/index.skills.tsv knowledge/index.paths.tsv knowledge/coverage.json knowledge/component-coverage.json

OBJS  := $(SRC:src/%.c=$(BUILD)/%.o)

RUN_TIMED    := $(if $(wildcard $(CURDIR)/tools/timer.py),python3 $(CURDIR)/tools/timer.py run,sh -c 'shift; exec "$$@"' _)
TIME_SUMMARY := $(if $(wildcard $(CURDIR)/tools/timer.py),python3 $(CURDIR)/tools/timer.py summary,true)
TIME_RESET   := $(if $(wildcard $(CURDIR)/tools/timer.py),python3 $(CURDIR)/tools/timer.py reset,true)

# A build from a Git checkout reports its commit in --version; packaged
# builds have no .git and report the plain version. Only the command's own
# paths count as dirty: a corpus-only edit must not change the command's
# embedded build id.
TAM_SOURCES := src bin completions Makefile source-build.json
TAM_BUILD_ID = $(or $(shell git -C "$(CURDIR)" log -n 1 --format=%h -- $(TAM_SOURCES) 2>/dev/null),$(shell git -C "$(CURDIR)" describe --always 2>/dev/null))$(if $(filter 1,$(TAM_SOURCE_DIRTY)),-dirty)
TAM_SOURCE_REVISION = $(or $(shell git -C "$(CURDIR)" log -n 1 --format=%H -- $(TAM_SOURCES) 2>/dev/null),$(shell git -C "$(CURDIR)" rev-parse HEAD 2>/dev/null))
TAM_SOURCE_DIRTY = $(if $(strip $(shell git -C "$(CURDIR)" status --porcelain -- $(TAM_SOURCES) 2>/dev/null)),1,0)

CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\"
IDENTITY_FLAGS = -DTAM_BUILD=\"$(TAM_BUILD_ID)\" -DTAM_SOURCE_REVISION=\"$(TAM_SOURCE_REVISION)\" -DTAM_SOURCE_DIRTY=$(TAM_SOURCE_DIRTY)
COMPILE_CONFIG = $(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS)
LINK_CONFIG = $(CC) $(LDFLAGS) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS)
LIBTAM_ARCHIVE = $(wildcard $(if $(LIBTAM_PREFIX),$(LIBTAM_PREFIX)/lib,$(shell $(PKG_CONFIG) --variable=libdir libtam 2>/dev/null))/libtam.a)
TAM_READER ?=
CORPUS_CACHE ?= 1
export CORPUS_CACHE TAM_READER SKIP_COVERAGE_CHECK
SESSION = $(if $(wildcard $(CURDIR)/tools/timer.py),python3 $(CURDIR)/tools/timer.py session,)

.PHONY: all command corpus corpus-check corpus-identity dev preview bench-corpus check-repo check-clean-tree test test-command corpus-and-test check install install-corpus uninstall clean libtam-check FORCE _test _test-command _command-checks _corpus-and-test _check corpus-tool-tests

# Whole project (default): the command and the corpus index.
all: command corpus

dev: ; @python3 tools/dev.py $(if $(DRY_RUN),--dry-run)

preview:
	@python3 tools/corpus_pipeline.py preview --page "$(PAGE)" $(if $(TAM_READER),--reader "$(TAM_READER)")

bench-corpus:
	@python3 tools/bench-corpus.py $(if $(TAM_READER),--reader "$(TAM_READER)")

# --- command lane: the C binary, no corpus regeneration -------------------
command: $(TAM)

$(BUILD)/%.o: src/%.c $(BUILD)/compile-config | $(BUILD) libtam-check
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS) -MMD -MP -c $< -o $@

# Only these objects consume changing checkout identity, keeping other objects warm.
$(BUILD)/tam.o $(BUILD)/upgrade.o: private CPPFLAGS += $(IDENTITY_FLAGS)
$(BUILD)/tam.o $(BUILD)/upgrade.o: $(BUILD)/identity-config

.SECONDEXPANSION:
$(TAM): $(OBJS) $$(LIBTAM_ARCHIVE) $(BUILD)/link-config | $(BUILD)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS) -o $@

$(BUILD)/compile-config: FORCE | $(BUILD)
	@printf '%s\n' '$(COMPILE_CONFIG)' | cmp -s - $@ || printf '%s\n' '$(COMPILE_CONFIG)' > $@

$(BUILD)/link-config: FORCE | $(BUILD)
	@printf '%s\n' '$(LINK_CONFIG)' | cmp -s - $@ || printf '%s\n' '$(LINK_CONFIG)' > $@

$(BUILD)/identity-config: FORCE | $(BUILD)
	@printf '%s\n' '$(IDENTITY_FLAGS)' | cmp -s - $@ || printf '%s\n' '$(IDENTITY_FLAGS)' > $@

-include $(OBJS:.o=.d)

$(BUILD):
	mkdir -p $(BUILD)

# Stop before the compiler does, with what to do instead of a missing tam.h.
libtam-check:
	@if [ -n '$(LIBTAM_ERROR)' ]; then \
		printf '%s\n' 'tam: cannot build the command: $(LIBTAM_ERROR).' \
		'  Install libtam 0.6.0 or later first, or set LIBTAM_PREFIX=<prefix>.' >&2; \
		exit 1; fi

# --- corpus lane: no compiler, libtam or command prerequisite --------------
corpus:
	@$(RUN_TIMED) "Corpus build" python3 tools/corpus_pipeline.py build

corpus-identity:
	@python3 tools/corpus_pipeline.py identity

corpus-check:
	@$(RUN_TIMED) "Corpus checks" python3 tools/corpus_pipeline.py check
	@$(RUN_TIMED) "Component coverage check" python3 tools/report-component-coverage.py --check --quiet

# Direct output requests share the same generation, including under make -j.
$(INDEX): corpus

# Session timing starts before recursive make expands any build prerequisites.
test:
	+@$(SESSION) $(MAKE) --no-print-directory _test

test-command:
	+@$(SESSION) $(MAKE) --no-print-directory _command-checks

corpus-and-test:
	+@$(SESSION) $(MAKE) --no-print-directory _corpus-and-test CORPUS_CACHE=0

check:
	+@$(SESSION) $(MAKE) --no-print-directory _check

_test: _test-command
	+@$(MAKE) --no-print-directory _corpus-and-test CORPUS_CACHE=0 TAM_READER="$(CURDIR)/$(TAM)"

_test-command: command
	@$(RUN_TIMED) "Command integration tests (tests/run.sh)" tests/run.sh $(CURDIR)/$(TAM)
	@$(RUN_TIMED) "Upgrade tests (test_upgrade.py)" python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q
	@$(RUN_TIMED) "Renderer golden tests (tests/render.sh)" tests/render.sh $(CURDIR)/$(TAM)

_command-checks: _test-command
	@$(RUN_TIMED) "Repository consistency check" python3 tools/check-repo.py

_corpus-and-test: corpus-check
	+@$(MAKE) --no-print-directory corpus-tool-tests
	@$(RUN_TIMED) "Repository consistency check" python3 tools/check-repo.py

corpus-tool-tests:
ifeq ($(SKIP_UNIT_TESTS),)
	@$(RUN_TIMED) "Corpus tooling unit tests" python3 -B -m unittest -q tests/test_lint_corpus.py tests/test_build_index.py tests/test_component_coverage.py tests/test_corpus_pipeline.py
else
	@printf '%s\n' 'tam: tooling unit tests skipped (SKIP_UNIT_TESTS).'
endif

check-repo:
	@if [ -d .git ] && [ -d .githooks ]; then git config core.hooksPath .githooks 2>/dev/null || true; fi
	python3 tools/check-repo.py

_check: _test
	@$(RUN_TIMED) "Developer and build guard unit tests" python3 -B -m unittest -q tests/test_dev.py tests/test_install_guard.py tests/test_check_repo.py tests/test_command_build.py

# A checkout installs only committed sources, so the installed --version names
# a commit rather than `-dirty`. The check runs after `all`, because rebuilding
# the corpus index can itself dirty the tree: the index records each page's
# commit history, so a page commit leaves it one commit behind until the
# follow-up index commit. Staged installs (DESTDIR) and builds without a .git
# of their own, such as the Nix package, are not checked.
INSTALL_SOURCES := $(TAM_SOURCES) knowledge tools
INDEX_FILES     := $(INDEX)

check-clean-tree:
	@[ -n "$(DESTDIR)" ] && exit 0; \
	top=$$(git -C $(CURDIR) rev-parse --show-toplevel 2>/dev/null) || exit 0; \
	[ "$$top" -ef "$(CURDIR)" ] || exit 0; \
	dirty=$$(git -C $(CURDIR) status --porcelain -- $(INSTALL_SOURCES)); \
	[ -z "$$dirty" ] && exit 0; \
	printf '%s\n' 'tam: refusing to install from a dirty tree:' >&2; \
	printf '%s\n' "$$dirty" | sed 's/^/  /' >&2; \
	if [ -z "$$(git -C $(CURDIR) status --porcelain -- $(INSTALL_SOURCES) ':!knowledge/index.*' ':!knowledge/coverage.json' ':!knowledge/component-coverage.json')" ]; then \
		printf '%s\n' 'Only the regenerated corpus index changed; commit it, then install again:' \
			"  git commit -m 'Refresh corpus index revisions after $$(git -C $(CURDIR) rev-parse --short HEAD)' -- $(INDEX_FILES)" >&2; \
	else \
		printf '%s\n' 'Commit or discard these changes first. Do not install another session'"'"'s uncommitted work.' >&2; \
	fi; \
	exit 1

install-corpus:
	@$(MAKE) --no-print-directory check-clean-tree
	rm -rf $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(DATADIR)
	cp -R knowledge/. $(DESTDIR)$(DATADIR)/

install: all
	@$(MAKE) --no-print-directory check-clean-tree
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TAM) $(DESTDIR)$(BINDIR)/tam
	install -m 755 bin/tam-man $(DESTDIR)$(BINDIR)/tam-man
	install -m 755 bin/tam-info $(DESTDIR)$(BINDIR)/tam-info
	install -m 755 bin/tam-tldr $(DESTDIR)$(BINDIR)/tam-tldr
	install -m 755 bin/tam-upgrade $(DESTDIR)$(BINDIR)/tam-upgrade
	rm -rf $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(DATADIR)
	cp -R knowledge/. $(DESTDIR)$(DATADIR)/
	install -d $(DESTDIR)$(MANDIR)/man1
	install -m 644 man/tam.1 $(DESTDIR)$(MANDIR)/man1/tam.1
	install -d $(DESTDIR)$(BASHCOMPDIR)
	install -m 644 completions/bash/tam $(DESTDIR)$(BASHCOMPDIR)/tam

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tam
	rm -f $(DESTDIR)$(BINDIR)/tam-man
	rm -f $(DESTDIR)$(BINDIR)/tam-info
	rm -f $(DESTDIR)$(BINDIR)/tam-tldr
	rm -f $(DESTDIR)$(BINDIR)/tam-upgrade
	rm -rf $(DESTDIR)$(DATADIR)
	rm -f $(DESTDIR)$(MANDIR)/man1/tam.1
	rm -f $(DESTDIR)$(BASHCOMPDIR)/tam

clean:
	rm -rf $(BUILD)
