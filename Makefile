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

LIBTAM_PREFIX ?= $(PREFIX)
LIBTAM_CFLAGS ?= -I$(LIBTAM_PREFIX)/include
LIBTAM_LIBS   ?= -L$(LIBTAM_PREFIX)/lib -ltam
RPATH         ?= -Wl,-rpath,$(LIBTAM_PREFIX)/lib

LDFLAGS  ?=
LDLIBS   ?=

BUILD := build
TAM   := $(BUILD)/tam
SRC   := src/tam.c

.PHONY: all clean test install uninstall

all: $(TAM)

$(TAM): $(SRC) src/version.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LIBTAM_CFLAGS) $(SRC) \
		$(LDFLAGS) $(RPATH) $(LIBTAM_LIBS) $(LDLIBS) -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(TAM)
	tests/run.sh $(CURDIR)/$(TAM)

install: $(TAM)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TAM) $(DESTDIR)$(BINDIR)/tam

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tam

clean:
	rm -rf $(BUILD)
