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
BASHCOMPDIR ?= $(PREFIX)/share/bash-completion/completions

LIBTAM_PREFIX ?= $(PREFIX)
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
RPATH         ?= -Wl,-rpath,$(LIBTAM_PREFIX)/lib

LDFLAGS  ?=
LDLIBS   ?=

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c src/render.c src/pager.c src/install.c src/man.c src/index.c src/depth.c src/ask.c
DEPS  := src/version.h src/render.h src/pager.h src/install.h src/man.h src/index.h src/depth.h src/ask.h
INDEX := knowledge/index.tsv knowledge/index.json

# A build from a Git checkout reports its commit in --version; packaged
# builds have no .git and report the plain version. Only the command's own
# paths count as dirty: a corpus-only edit must not change the command's
# embedded build id.
TAM_SOURCES := src bin completions Makefile
TAM_BUILD_ID := $(shell git -C $(CURDIR) describe --always 2>/dev/null)
ifneq ($(strip $(shell git -C $(CURDIR) status --porcelain -- $(TAM_SOURCES) 2>/dev/null)),)
TAM_BUILD_ID := $(TAM_BUILD_ID)-dirty
endif

CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\" -DTAM_BUILD=\"$(TAM_BUILD_ID)\"

# The binary embeds DATADIR, the libtam rpath and the build id, so a build
# made for one PREFIX must not be installed under another. The stamp changes
# whenever that configuration does and forces a rebuild.
CONFIG := $(CC) $(CPPFLAGS) $(CFLAGS) $(LIBTAM_CFLAGS) $(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS)

.PHONY: all command corpus test test-command test-corpus check install uninstall clean FORCE

# Whole project (default): the command and the corpus index.
all: command corpus

# --- command lane: the C binary, no corpus regeneration -------------------
command: $(TAM)

$(TAM): $(SRC) $(DEPS) $(BUILD)/config | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LIBTAM_CFLAGS) $(SRC) \
		$(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS) -o $@

$(BUILD)/config: FORCE | $(BUILD)
	@printf '%s\n' '$(CONFIG)' | cmp -s - $@ || printf '%s\n' '$(CONFIG)' > $@

$(BUILD):
	mkdir -p $(BUILD)

# --- corpus lane: index generation and validation -------------------------
corpus: $(INDEX)

$(INDEX): $(wildcard knowledge/*.html) $(wildcard knowledge/*/*.html) tools/build-index.py
	python3 tools/build-index.py

# --- tests ----------------------------------------------------------------
# test-command and test-corpus are single-lane entry points; test and check
# are the full, integrated runs.
test: all
	tests/run.sh $(CURDIR)/$(TAM)
	tests/guide.sh $(CURDIR)/$(TAM)

test-command: command
	tests/run.sh $(CURDIR)/$(TAM)

test-corpus: command
	tests/guide.sh $(CURDIR)/$(TAM)

check: corpus test

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TAM) $(DESTDIR)$(BINDIR)/tam
	install -m 755 bin/tam-man $(DESTDIR)$(BINDIR)/tam-man
	rm -rf $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(DATADIR)
	cp -R knowledge/. $(DESTDIR)$(DATADIR)/
	install -d $(DESTDIR)$(BASHCOMPDIR)
	install -m 644 completions/bash/tam $(DESTDIR)$(BASHCOMPDIR)/tam

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tam
	rm -f $(DESTDIR)$(BINDIR)/tam-man
	rm -rf $(DESTDIR)$(DATADIR)
	rm -f $(DESTDIR)$(BASHCOMPDIR)/tam

clean:
	rm -rf $(BUILD)
