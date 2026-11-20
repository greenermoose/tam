# Help message redesign; `tam inspect` and `tam check` at the top level

- **Date:** 2026-10-14
- **Stage:** Develop — help tiers and command promotion, between tam 0.21.0 and 0.22.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5, from the agy process log that served this session.
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commit:** `9464b0f`, which carries no AI trailers.
- **Record:** Written on 2026-10-15 by Claude Code from the session transcript; the session wrote no record of its own.
- **Authorship:** Fred designed the help text with Antigravity in a review document, edited it, and decided the command changes. Antigravity drafted the messages in `docs/plans/help-message-redesign.md` and implemented them.

## Guiding prompts

Session-management questions are omitted. The help output Fred pasted in
the first prompt is kept as he sent it.

> Let's discuss the tam command and think more about the subcommands that are available. I thought we had finished tam ask but I don't see it what I run tam. Here's what I get:
>
> tam - the Tamlinux command
>
> Learn about this system, keep it healthy, and find your way
> around Tamlinux, offline, from the terminal.
>
> This is an early development build (tam 0.21.0).
>
> Get started:
>   tam welcome          open the welcome orientation
>   tam show             open the offline guide
>   tam search TERM...   search the offline guide
>   tam explain [TOPIC]  explain a topic or list available topics
>   tam install inspect  inspect host and installation capabilities
>   tam --help           list options and where to begin
>   tam --version        show version information
>
> What's missing is tam ask [TOPIC]. Also, I thought tam show [PAGE] would work, but it says the form of the subcommand is tam show.
>
> Please help me think through the difference in output between
>
> tam
>
> and
>
> tam --help
>
> Why the difference? Should plain tam do something different?

> Option 1. I want to work with you on the exact help message, though.

> I want the organization of Draft B but I want the complete examples of Draft A. E.g.,
>
> tam - the Tamlinux command
>
> Learn how to use, maintain, and improve your system.
>
> This is version x.y.z (build 000000)
>
> Usage:
>   tam <command> [--options] [arguments]
>
> Guide and knowledge:
>   tam welcome               first-run orientation
>   tam show [PAGE]           open the guide or a specific page
>   tam explain [TOPIC]       browse topics or explain a concept
>
> etc.
>
> Just plain tam should not list all commands, options, and arguments, but
>
> tam --verbose
> tam --help --verbose
> tam help --verbose
>
> should all be synonyms for a complete usage message that shows all commands and options.
>
> Options:
>   --help, -h
>   --pipe
>   --terse
>   --verbose
>   --version, -V
>
> Make sure to list all options in alphabetical order.
>
> Please create a set of messages for tam --terse, tam, and tam --verbose so I can review. Thank you!

> Please put these in a doc and show it to me in micro so I can review and edit.

> yes

> I have made some changes to the file. In particular, note that I'd like to change tam install inspect and tam install check to be tam inspect and tam check. I don't think install is the right verb for what we're doing there. I may bring back tam install for installing additional Tamlinux system components. But I think we want tam inspect and tam check now. Ask if you have any questions.

> Regarding backward compatibility, no to keeping tam install inspect and tam install check as aliases. Retire them entirely. We will rewrite scripts and docs. When someone runs bare tam install, show the installer framework overview. We will make that work for components in the future. Yes, tab completion should complete inspect and check at the top level.
>
> Please close the micro-review window. I am happy wtih the plan. Begin implementing 1 through 3 in your list.
>
> Any questions?

## Decisions and scope

- Three help tiers: `--terse`, the default, and `--verbose` (also
  `--help --verbose` and `help --verbose`), with options in alphabetical
  order.
- `tam inspect` and `tam check` replace `tam install inspect` and
  `tam install check` with no aliases; bare `tam install` shows the
  installer framework overview. Bash completion offers both at the top
  level.

## Verification

The transcript shows `tests/run.sh` passing after the change; the guide
suite was not run in this session.
