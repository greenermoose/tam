# Tam Command Help Message Redesign

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Implemented help tiers and top-level inspect/check, between command 0.21.0 and 0.22.0.
**Authority:** [Current development contract](development.md) and the evidence below.
**Next action:** None for this closed plan; follow the current roadmap.
**Completion evidence:** Implementation `9464b0f`; [developer decisions and verification](../ai/2026-10-14-tam-help-message-redesign.md). The recorded final decisions, including no legacy inspect/check aliases, override draft examples below. Update new navigation help under the [overhaul](knowledge-system-0.31.0.md).

> Historical design snapshot. This plan is closed; its original work items do
> not assign further implementation. Current behavior belongs to the development
> contract and current scope to the [plan index](README.md).

## Original design record

**Date:** 2026-10-14  
**Target:** `tam` (Milestone 14 / Tamlinux 0.22.0)  

---

## 1. Context and Goals

1. **Unify Bare Invocation and `--help`:**
   Running `tam` without arguments and running `tam --help` (or `tam help`) should no longer present divergent, drifting lists of capabilities.
2. **Progressive Disclosure (Three Tiers):**
   - **Terse (`tam --terse`):** Ultra-compact summary for narrow windows or quick command lookup.
   - **Standard (`tam` / `tam --help`):** Clean, categorized command reference with common argument forms and alphabetically ordered options.
   - **Verbose (`tam --verbose` / `tam --help --verbose` / `tam help --verbose`):** Exhaustive specification including all secondary verbs, argument alternates, flags, and environment variables.
3. **Accurate Command Representation:**
   - Clearly document `tam show [PAGE]` (not just `tam show`).
   - Restore visibility for `tam ask QUESTION` and `tam man [SECTION] PAGE`.
4. **Alphabetical Option Ordering:**
   Options sorted alphabetically by long name: `--help`, `--pipe`, `--terse`, `--verbose`, `--version`.

---

## 2. Standard Help: `tam` and `tam --help`

Invoked via:
- `tam`
- `tam --help`, `tam -h`, `tam help`

```text
tam - the Tamlinux command

Understand, use, maintain, and improve your Tamlinux system.

This is version 0.21.0 (dev 98011df)

Usage:
  tam <subcommand> [--options] [arguments]

  tam welcome               first-run orientation

Guide subcommands to try:
  tam show [PAGE]           open the guide or a specific page
  tam search KEYWORD...     search guide deterministically
  tam explain [TOPIC]       browse all topics or show a topic
  tam man [SECTION] PAGE    view hyperlinked manual page
  tam ask QUESTION          ask a local AI model (if available)

System subcommands to try:
  tam inspect               inspect system components
  tam check                 check system health

Options to know:
  --help, -h                show this help overview
  --pipe                    output as plain text for piping
  --terse                   shorter output
  --verbose                 longer output
  --version, -V             show version info

Run 'tam --verbose' for a complete list of commands and more detail.
```

---

## 3. Terse Help: `tam --terse`

Invoked via:
- `tam --terse`
- `tam --help --terse`, `tam -h --terse`, `tam help --terse`

```text
tam - the Tamlinux command

Usage: tam <subcommand> [--options] [arguments]

Essential Subcommands:
  tam welcome                   first-run orientation
  tam show [PAGE]               open guide or specific page
  tam inspect                   inspect system health

Essential Options:
  --help, -h | --pipe | --terse | --verbose | --version, -V
```

---

## 4. Verbose Help: `tam --verbose`

Invoked via:
- `tam --verbose`
- `tam --help --verbose`, `tam -h --verbose`, `tam help --verbose`

```text
tam - the Tamlinux command

Understand, use, maintain, and improve your Tamlinux system.

This is version 0.21.0 (dev 98011df)

Usage:
  tam <subcommand> [--options] [arguments]

Guide and knowledge:
  tam welcome [--pipe]
      first-run orientation and welcome guide
  tam show [PAGE|-] [--pipe]
      open the offline guide, a specific page, or render HTML from stdin
  tam search [--json] TERM...
      search guide topics and keywords with ranked scoring
  tam explain [TOPIC] [--level LEVEL] [--pipe]
      explain a concept or browse the catalog at beginner, intermediate, or expert depth
  tam explain topics [--pipe] [--terse|--verbose] [--level LEVEL]
      interactive or plain-text catalog of all available topics
  tam man [SECTION] PAGE [--pipe]
      hyperlinked manual page viewer with interactive cross-reference navigation
  tam ask [--json] [--model MODEL] [--server URL] QUESTION
      query a local AI model for answers from the guide to any question

Inquiries and health:
  tam inspect [--json|--check [ID]] [--terse]
      inspect hardware, kernel, DRM, storage mounts, and privilege escalation
  tam check [ID] [--json|--terse]
      evaluate workstation health findings against the offline Check Contract

Options:
  --help, -h
      display help overview (combine with --verbose for this exhaustive view)
  --pipe
      emit clean plain text without ANSI escape sequences or interactive pager
  --terse
      emit compact single-line or summary output
  --verbose
      show all subcommands, arguments, secondary verbs, and options
  --version, -V
      display command, libtam, and Tamlinux cultivar metadata

Environment:
  TAM_KNOWLEDGE_DIR         override path to offline HTML guide corpus
  TAM_ASK_MODEL             default local LLM name (default: qwen2.5-coder:1.5b)
  TAM_ASK_SERVER            default local inference URL (default: http://127.0.0.1:11434)

Run 'tam <subcommand> --help' for options specific to any subcommand.
```

---

## 5. Implementation Plan

1. **`src/tam.c` Dispatch and Formatting:**
   - Update `main()` argument parsing to recognize `--verbose` and `--terse` flags combined with bare invocation, `--help`, or `help`.
   - Replace separate, drifting `print_intro()` and `print_help()` implementations with a parameterized printer function `print_help_level(int level)` (where `level` is `HELP_TERSE`, `HELP_STANDARD`, or `HELP_VERBOSE`).
   - Format version line dynamically using `TAM_COMMAND_VERSION` and `TAM_BUILD`.
2. **Test Updates (`tests/run.sh`):**
   - Ensure `tests/run.sh` verifies `tam --help`, `tam`, `tam --terse`, and `tam --verbose`.
   - Update line-count assertion if standard help is ~23 lines (or keep standard help at <= 20 lines if strict 80x24 terminal constraint is preserved).
3. **Manual Page (`man/tam.1`):**
   - Synchronize command synopsis and subcommands section with the redesign.
