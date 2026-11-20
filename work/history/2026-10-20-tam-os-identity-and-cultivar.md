# Operating system and Tamlinux cultivar identity (command 0.35.0, corpus 0.36.1)

- **Date:** 2026-10-20
- **Time:** about 19:30 to 20:45 -04:00
- **Stage:** Develop. `tam inspect` and `tam --version` report the
  distribution and the Tamlinux cultivar separately; two new findings.
- **CLI Tool:** Claude Code 2.1.305
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `f9cf35d`, `f413950` and the commit that adds this record
- **Authorship:** Fred Horch asked for a review of the version-numbering and
  operating-system-name plans and decided each recommendation. Claude wrote
  the command, test, corpus and plan changes.
- **Checked against:** `src/install.c`, `src/tam.c`, `tests/run.sh`,
  `/etc/os-release` and `/usr/lib/os-release` on the workstation, the
  `tamlinux` repository `VERSIONING.md` and `CULTIVAR`, and the
  `tamlinux-packages` shell module that installs the cultivar file

## Guiding prompts

> Review the plans for Tamlinux version numbers without letters starting with
> version 0.4.5 and the plans for the name and version of the operating system
> reported by tam inspect and other tools. Are they all sound? Anything you
> would change?

> Let's name the 0.4 cultivar "Incipiens". So the tam inspect rows should
> show:
>
> Operating system: Arch Linux (rolling)
> Tamlinux cultivar: Incipiens 0.4.4-m
>
> For 0.5, the cultivar name will be Blue sparkler.

> I made up incipiens from the Latin for "to begin, undertake" signifying that
> we are beginning the project. This is an exception to the rule that the
> cultivar names be real. Your other suggestions are fine. What's next?

> Make the change, then continue with step 4.

## Key decisions

- Fred chose to keep Arch Linux as the operating system name and report
  Tamlinux as the cultivar that runs on it, rather than ship a Tamlinux
  os-release (Tamlinux decision D181).
- The version and cultivar name come from the invoking user's files, since
  Tamlinux is deployed per user; under `pkexec` the user is `PKEXEC_UID`.
- The version file still wins over `$TAMLINUX_VERSION`. The decision text
  lists the environment first; the existing order was kept because activation
  rewrites the file while a running process keeps its environment. This
  disagreement is reported to the developer.
- `cultivar_version` stays in the JSON for existing readers.

## Verification

`make test`: command, corpus and guide suites pass, including new cases for
the cultivar name, the not-deployed row and finding, and fixture
os-release trees for a plain Arch base and an override. On the workstation,
`build/tam inspect` flags the inherited `/etc/os-release` (`ID=omarchy` over
`ID=arch`).
