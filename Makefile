# tam - the Tamlinux command
#
# Builds the command against an installed libtam. The library is a separate
# dependency and is never built or modified by this Makefile.
#
# libtam 0.4.0 or later is linked statically. It is found with
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
PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin
DATADIR  ?= $(PREFIX)/share/tamlinux/knowledge
MANDIR   ?= $(PREFIX)/share/man
BASHCOMPDIR ?= $(PREFIX)/share/bash-completion/completions

PKG_CONFIG ?= pkg-config
ifdef LIBTAM_PREFIX
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
LIBTAM_ERROR  := $(if $(shell grep -s -l tam_exec_opts $(LIBTAM_PREFIX)/include/tam.h),,LIBTAM_PREFIX=$(LIBTAM_PREFIX) has no libtam 0.4.0 or later include/tam.h)
else ifeq ($(origin LIBTAM_LIBS),undefined)
LIBTAM_FOUND  := $(shell $(PKG_CONFIG) --atleast-version=0.4.0 libtam 2>/dev/null && echo yes)
LIBTAM_CFLAGS := $(if $(LIBTAM_FOUND),$(shell $(PKG_CONFIG) --cflags libtam))
LIBTAM_LIBS   := $(if $(LIBTAM_FOUND),$(shell $(PKG_CONFIG) --libs libtam))
LIBTAM_ERROR  := $(if $(LIBTAM_FOUND),,$(PKG_CONFIG) cannot find libtam 0.4.0 or later)
endif

LDFLAGS  ?=
LDLIBS   ?= -lz
TERMINFO_CFLAGS ?= $(shell pkg-config --cflags ncursesw 2>/dev/null)
TERMINFO_LIBS   ?= $(shell pkg-config --libs ncursesw 2>/dev/null || echo -ltinfo)

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c src/render.c src/display.c src/pager.c src/pager_screen.c src/terminal.c src/install.c src/man.c src/index.c src/depth.c src/fold.c src/ask.c src/info.c src/discovery.c src/tldr.c src/upgrade.c
DEPS  := src/version.h src/render.h src/display.h src/pager.h src/pager_screen.h src/terminal.h src/install.h src/man.h src/index.h src/depth.h src/fold.h src/ask.h src/info.h src/discovery.h src/tldr.h src/upgrade.h
INDEX := knowledge/index.tsv knowledge/index.json knowledge/index.parents.tsv knowledge/index.kinds.tsv knowledge/index.skills.tsv knowledge/coverage.json

# A build from a Git checkout reports its commit in --version; packaged
# builds have no .git and report the plain version. Only the command's own
# paths count as dirty: a corpus-only edit must not change the command's
# embedded build id.
TAM_SOURCES := src bin completions Makefile source-build.json
TAM_BUILD_ID := $(shell git -C $(CURDIR) describe --always 2>/dev/null)
TAM_SOURCE_REVISION := $(shell git -C $(CURDIR) rev-parse HEAD 2>/dev/null)
TAM_SOURCE_DIRTY := $(if $(strip $(shell git -C $(CURDIR) status --porcelain -- $(TAM_SOURCES) 2>/dev/null)),1,0)
ifneq ($(strip $(shell git -C $(CURDIR) status --porcelain -- $(TAM_SOURCES) 2>/dev/null)),)
TAM_BUILD_ID := $(TAM_BUILD_ID)-dirty
endif

CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\" -DTAM_BUILD=\"$(TAM_BUILD_ID)\"
CPPFLAGS += -DTAM_SOURCE_REVISION=\"$(TAM_SOURCE_REVISION)\"
CPPFLAGS += -DTAM_SOURCE_DIRTY=$(TAM_SOURCE_DIRTY)

# The binary embeds DATADIR, the static libtam and the build id, so a build
# made for one PREFIX must not be installed under another. The stamp changes
# whenever that configuration does and forces a rebuild.
CONFIG := $(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS) $(LDFLAGS) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS)

.PHONY: all command corpus dev check-repo check-clean-tree test test-command corpus-and-test check install uninstall clean libtam-check FORCE

# Whole project (default): the command and the corpus index.
all: command corpus

dev: ; @python3 tools/dev.py $(if $(DRY_RUN),--dry-run)

# --- command lane: the C binary, no corpus regeneration -------------------
command: $(TAM)

$(TAM): $(SRC) $(DEPS) $(BUILD)/config | $(BUILD) libtam-check
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS) $(SRC) \
		$(LDFLAGS) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS) -o $@

$(BUILD)/config: FORCE | $(BUILD)
	@printf '%s\n' '$(CONFIG)' | cmp -s - $@ || printf '%s\n' '$(CONFIG)' > $@

$(BUILD):
	mkdir -p $(BUILD)

# Stop before the compiler does, with what to do instead of a missing tam.h.
libtam-check:
ifneq ($(LIBTAM_ERROR),)
	@printf '%s\n' 'tam: cannot build the command: $(LIBTAM_ERROR).' \
		'  Install libtam 0.4.0 or later first:   make -C <libtam checkout> install PREFIX=<prefix>' \
		'  then let pkg-config see it:   export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig' \
		'  or name the prefix:   make LIBTAM_PREFIX=<prefix>' >&2
	@exit 1
endif

# --- corpus lane: index generation and validation -------------------------
# Git history and page removals can change the index without changing an input
# timestamp. Always recheck; the generator preserves unchanged output files.
corpus:
	python3 tools/build-index.py

# Direct output requests share the same generation, including under make -j.
$(INDEX): corpus

# --- tests ----------------------------------------------------------------
# test-command and corpus-and-test are single-lane entry points; test and check
# are the full, integrated runs.
test: all
	tests/run.sh $(CURDIR)/$(TAM)
	TAM_TEST_SKIP_OLLAMA=1 python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q
	tests/render.sh $(CURDIR)/$(TAM)
	python3 tools/lint-corpus.py
	python3 -m unittest -q tests/test_lint_corpus.py
	python3 -m unittest -q tests/test_build_index.py
	python3 -m unittest -q tests/test_dev.py
	python3 -m unittest -q tests/test_install_guard.py
	python3 tools/check-repo.py
	python3 -m unittest -q tests/test_check_repo.py
	tests/guide.sh $(CURDIR)/$(TAM)

check-repo:
	python3 tools/check-repo.py

test-command: command
	tests/run.sh $(CURDIR)/$(TAM)
	TAM_TEST_SKIP_OLLAMA=1 python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q
	tests/render.sh $(CURDIR)/$(TAM)

corpus-and-test: command corpus
	python3 tools/lint-corpus.py
	python3 -m unittest -q tests/test_lint_corpus.py
	python3 -m unittest -q tests/test_build_index.py
	tests/guide.sh $(CURDIR)/$(TAM)

check: corpus test

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
	if [ -z "$$(git -C $(CURDIR) status --porcelain -- $(INSTALL_SOURCES) ':!knowledge/index.*' ':!knowledge/coverage.json')" ]; then \
		printf '%s\n' 'Only the regenerated corpus index changed; commit it, then install again:' \
			"  git commit -m 'Refresh corpus index revisions after $$(git -C $(CURDIR) rev-parse --short HEAD)' -- $(INDEX_FILES)" >&2; \
	else \
		printf '%s\n' 'Commit or discard these changes first. Do not install another session'"'"'s uncommitted work.' >&2; \
	fi; \
	exit 1

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
