#!/usr/bin/env python3
"""Shared CLI-aware token rules for corpus search.

Search terms are single CLI words. Ordinary words are stemmed with the same
small suffix rules as the reader; flags (--pipe), dotted IDs (shell.novice),
version strings (0.71.0), paths and identifiers keep their punctuation so a
query can name exactly what the reader accepts on the command line. The C
reader mirrors these rules in src/index.c; do not change one without the other.
"""

from __future__ import annotations

import re

STOP_WORDS = frozenset({
    "a", "an", "the", "and", "or", "of", "to", "in", "on", "for",
    "how", "do", "does", "you", "what", "why", "i", "me", "my",
    "is", "are", "it", "its", "that", "this", "with", "at", "by",
    "from", "be", "can", "get", "run", "using", "use",
})

MAX_QUERY_TOKENS = 16
VERSION_RE = re.compile(r"^\d+(?:\.\d+)+(?:[-+][A-Za-z0-9._-]+)*$")
KINDS = frozenset({"word", "flag", "version", "id", "path"})


def _is_start(char):
    return char.isalnum() or char in "-_/"


def _is_continue(char):
    return char.isalnum() or char in "-._/+"


def _token_kind(raw):
    if raw.startswith("--") or (raw.startswith("-") and len(raw) > 1 and raw[1].isalnum()):
        return "flag"
    if VERSION_RE.match(raw):
        return "version"
    if "/" in raw:
        return "path"
    if "." in raw or "_" in raw:
        return "id"
    return "word"


def stem_word(raw, kind=None):
    """Lower-case and, for ordinary words only, apply the reader's suffix rules."""
    word = raw.lower()
    kind = kind or _token_kind(raw)
    if kind != "word":
        return word
    if len(word) > 4 and word.endswith("ies"):
        return word[:-3]
    if len(word) > 4 and word.endswith("ing"):
        return word[:-3]
    if len(word) > 3 and word.endswith("ed"):
        return word[:-2]
    if len(word) > 3 and word.endswith("es"):
        return word[:-2]
    if len(word) > 3 and word.endswith("s"):
        return word[:-1]
    return word


def _clean(raw):
    # A bullet's lone dash is not a flag; leading dashes stay only when they
    # introduce a flag (--pipe, -h).
    flag_prefix = raw.startswith("--") and len(raw) > 2 and raw[2].isalnum()
    while (not flag_prefix and len(raw) > 1 and raw[0] == "-"
           and not raw[1].isalnum()):
        raw = raw[1:]
    while raw and raw[-1] in ".-/":
        raw = raw[:-1]
    if not raw or all(char in "-._/+" for char in raw):
        return ""
    return raw


def tokenize(text, max_tokens=MAX_QUERY_TOKENS):
    """Return {raw, stem, kind} tokens in encounter order.

    As in the reader, a query made only of stop words keeps them; otherwise
    stop words are dropped so 'how do I' does not swamp the useful terms.
    """
    tokens = []
    index = 0
    length = len(text)
    while index < length and len(tokens) < max_tokens:
        while index < length and not _is_start(text[index]):
            index += 1
        start = index
        while index < length and _is_continue(text[index]):
            index += 1
        raw = _clean(text[start:index])
        if not raw:
            continue
        lowered = raw.lower()
        if any(existing["raw"].lower() == lowered for existing in tokens):
            continue
        kind = _token_kind(raw)
        tokens.append({"raw": raw, "stem": stem_word(raw, kind), "kind": kind})

    meaningful = [token for token in tokens if token["raw"].lower() not in STOP_WORDS]
    return meaningful if meaningful else tokens


def single_token(value):
    """Validate authored boost/exclude metadata as exactly one CLI token."""
    tokens = tokenize(value.strip(), max_tokens=2)
    if len(tokens) != 1 or tokens[0]["raw"].lower() != value.strip().lower():
        return None
    if tokens[0]["raw"].lower() in STOP_WORDS:
        return None
    return tokens[0]
