# tam - the Tamlinux command
#
# Builds the command against an installed libtam. The library is a separate
# dependency and is never built or modified by this Makefile.
#
# Override LIBTAM_PREFIX if libtam is installed somewhere other than PREFIX,
# or LIBTAM_CFLAGS / LIBTAM_LIBS directly for a custom location.

CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra -Wpedantic -std=c11
PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin
DATADIR  ?= $(PREFIX)/share/tamlinux/knowledge

LIBTAM_PREFIX ?= $(PREFIX)
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
RPATH         ?= -Wl,-rpath,$(LIBTAM_PREFIX)/lib

LDFLAGS  ?=
LDLIBS   ?=

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c src/render.c src/pager.c src/install.c src/man.c
DEPS  := src/version.h src/render.h src/pager.h src/install.h src/man.h

# A build from a Git checkout reports its commit in --version; packaged
# builds have no .git and report the plain version.
TAM_BUILD_ID := $(shell git -C $(CURDIR) describe --always --dirty 2>/dev/null)

CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\" -DTAM_BUILD=\"$(TAM_BUILD_ID)\"

# The binary embeds DATADIR, the libtam rpath and the build id, so a build
# made for one PREFIX must not be installed under another. The stamp changes
# whenever that configuration does and forces a rebuild.
CONFIG := $(CC) $(CPPFLAGS) $(CFLAGS) $(LIBTAM_CFLAGS) $(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS)

.PHONY: all clean test install uninstall FORCE

all: $(TAM)

$(TAM): $(SRC) $(DEPS) $(BUILD)/config | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LIBTAM_CFLAGS) $(SRC) \
		$(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS) -o $@

$(BUILD)/config: FORCE | $(BUILD)
	@printf '%s\n' '$(CONFIG)' | cmp -s - $@ || printf '%s\n' '$(CONFIG)' > $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(TAM)
	tests/run.sh $(CURDIR)/$(TAM)
	tests/guide.sh $(CURDIR)/$(TAM)

install: $(TAM)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TAM) $(DESTDIR)$(BINDIR)/tam
	install -m 755 bin/tam-man $(DESTDIR)$(BINDIR)/tam-man
	rm -rf $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(DATADIR)
	cp -R knowledge/. $(DESTDIR)$(DATADIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tam
	rm -f $(DESTDIR)$(BINDIR)/tam-man
	rm -rf $(DESTDIR)$(DATADIR)

clean:
	rm -rf $(BUILD)
