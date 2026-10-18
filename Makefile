# tam - the Tamlinux command
#
# Builds the command against an installed libtam. The library is a separate
# dependency and is never built or modified by this Makefile.
#
# Override LIBTAM_PREFIX if libtam is installed somewhere other than PREFIX,
# or LIBTAM_CFLAGS / LIBTAM_LIBS directly for a custom location.
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

LIBTAM_PREFIX ?= $(PREFIX)
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
RPATH         ?= -Wl,-rpath,$(LIBTAM_PREFIX)/lib

LDFLAGS  ?=
LDLIBS   ?= -lz
TERMINFO_CFLAGS ?= $(shell pkg-config --cflags ncursesw 2>/dev/null)
TERMINFO_LIBS   ?= $(shell pkg-config --libs ncursesw 2>/dev/null || echo -ltinfo)

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c src/render.c src/display.c src/pager.c src/pager_screen.c src/terminal.c src/install.c src/man.c src/index.c src/depth.c src/ask.c src/info.c src/discovery.c src/upgrade.c
DEPS  := src/version.h src/render.h src/display.h src/pager.h src/pager_screen.h src/terminal.h src/install.h src/man.h src/index.h src/depth.h src/ask.h src/info.h src/discovery.h src/upgrade.h
INDEX := knowledge/index.tsv knowledge/index.json knowledge/index.parents.tsv

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

# The binary embeds DATADIR, the libtam rpath and the build id, so a build
# made for one PREFIX must not be installed under another. The stamp changes
# whenever that configuration does and forces a rebuild.
CONFIG := $(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS) $(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS)

.PHONY: all command corpus test test-command test-corpus check install uninstall clean FORCE

# Whole project (default): the command and the corpus index.
all: command corpus

# --- command lane: the C binary, no corpus regeneration -------------------
command: $(TAM)

$(TAM): $(SRC) $(DEPS) $(BUILD)/config | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TERMINFO_CFLAGS) $(LIBTAM_CFLAGS) $(SRC) \
		$(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS) $(TERMINFO_LIBS) -o $@

$(BUILD)/config: FORCE | $(BUILD)
	@printf '%s\n' '$(CONFIG)' | cmp -s - $@ || printf '%s\n' '$(CONFIG)' > $@

$(BUILD):
	mkdir -p $(BUILD)

# --- corpus lane: index generation and validation -------------------------
corpus: $(INDEX)

$(INDEX): $(wildcard knowledge/*.html) $(wildcard knowledge/*/*.html) knowledge/version tools/build-index.py
	python3 tools/build-index.py

# --- tests ----------------------------------------------------------------
# test-command and test-corpus are single-lane entry points; test and check
# are the full, integrated runs.
test: all
	tests/run.sh $(CURDIR)/$(TAM)
	TAM_TEST_SKIP_OLLAMA=1 python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q
	tests/render.sh $(CURDIR)/$(TAM)
	python3 tools/lint-corpus.py
	python3 -m unittest -q tests/test_lint_corpus.py
	python3 -m unittest -q tests/test_build_index.py
	tests/guide.sh $(CURDIR)/$(TAM)

test-command: command
	tests/run.sh $(CURDIR)/$(TAM)
	TAM_TEST_SKIP_OLLAMA=1 python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q
	tests/render.sh $(CURDIR)/$(TAM)

test-corpus: command
	python3 tools/lint-corpus.py
	python3 -m unittest -q tests/test_lint_corpus.py
	python3 -m unittest -q tests/test_build_index.py
	tests/guide.sh $(CURDIR)/$(TAM)

check: corpus test

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TAM) $(DESTDIR)$(BINDIR)/tam
	install -m 755 bin/tam-man $(DESTDIR)$(BINDIR)/tam-man
	install -m 755 bin/tam-info $(DESTDIR)$(BINDIR)/tam-info
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
	rm -f $(DESTDIR)$(BINDIR)/tam-upgrade
	rm -rf $(DESTDIR)$(DATADIR)
	rm -f $(DESTDIR)$(MANDIR)/man1/tam.1
	rm -f $(DESTDIR)$(BASHCOMPDIR)/tam

clean:
	rm -rf $(BUILD)
