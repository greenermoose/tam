# Weblate

## What it is

- **Format:** a web-based localisation platform with three documented workflows: direct translation, peer review by vote, and dedicated reviewers who approve strings ([Weblate translation workflows, full page read, accessed 2026-11-26](https://docs.weblate.org/en/latest/workflows.html)).
- **Licence and scope:** translation management, per-string states, source-string feedback and commit gating; no licence statement was checked.
- **Evidence:** Weblate workflow, review and basics pages were read directly; KDE and GNOME context comes from project pages and archived material. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Per-string states are actionable: `Needs editing` and `Needs rewriting` say what is wrong, unlike a generic "outdated" label ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
  Tam application: use a small set of reasoned freshness states in the corpus index, with "source changed" as a trigger.
- **adopt**: A quality filter, not merely enabled reviews, is the gateway that keeps unapproved translations out of commits ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
  Tam application: for any generated text, define an explicit acceptance gate before publication.
- **adopt**: Translator comments on source strings can be routed to a source-string issue address, and source reviews can mark a string for correction ([Weblate reviewing strings, full page read, accessed 2026-11-26](https://docs.weblate.org/en/latest/devel/review.html)).
  Tam application: a reader complaint may indicate a bad source page, not a bad derivative; route feedback accordingly.
- **consider**: Multiple failing checks on one source string signal that translators struggle with it and the source should be reviewed ([Weblate reviewing strings](https://docs.weblate.org/en/latest/devel/review.html)).
  Tam application: repeated corrections against one page can trigger a source review.
- **consider**: Parent-child language rules can make derived work read-only until the parent no longer needs editing ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
  Tam application: only relevant with real translated derivatives, where the parent must no longer need editing first; model the state, not the platform.
- **consider**: Weblate distinguishes human-assisted drafts, service translations with human editing and service suggestions with human acceptance ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
  Tam application: a draft-versus-approved distinction is useful even without translation.
- **reject**: Weblate itself for an English-only corpus with no evidenced translation demand ([KDE Discuss, full page read, accessed 2026-11-26](https://discuss.kde.org/t/any-plans-to-move-to-weblate-for-translation/3415)).
  Tam application: adding a platform is a much larger commitment than adding metadata.
- **reject**: Publication of unreviewed machine-generated text: AI evaluation records checks but does not replace approval or change the commit gateway ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
  Tam application: AI assistance in drafting is acceptable; human verification before publication is not optional.
- **reject**: Publishing translator or contributor email in version control without care: Weblate warns that published commit email is effectively irrevocable ([Weblate basics, full page read, accessed 2026-11-26](https://docs.weblate.org/en/latest/user/basic.md)).
  Tam application: keep private contact details out of public records.

## Things to avoid

- Do not claim enabling reviews alone blocks unapproved commits: Weblate explicitly says a quality filter is needed ([Weblate workflows](https://docs.weblate.org/en/latest/workflows.html)).
- Do not say KDE moved to Weblate: the 2023 answer was no ([KDE Discuss](https://discuss.kde.org/t/any-plans-to-move-to-weblate-for-translation/3415)).
- Do not treat archived GNOME translation-role pages as current 2026 practice ([archived GNOME Translation Project, accessed 2026-11-26](https://wiki.gnome.org/TranslationProject/ContributeTranslations)).
- Do not assume translation is needed at all; none was evidenced for this corpus.

## Where to look

- All URLs below were accessed 2026-11-26; archived pages are marked.
- [Full page read] [Weblate translation workflows](https://docs.weblate.org/en/latest/workflows.html) — workflows, states, quality filter and AI distinctions.
- [Full page read] [Weblate reviewing strings](https://docs.weblate.org/en/latest/devel/review.html) — source-string feedback and failing checks.
- [Full page read] [Weblate basics](https://docs.weblate.org/en/latest/user/basic.md) — commit attribution and privacy.
- [Full page read] [KDE Discuss: Weblate](https://discuss.kde.org/t/any-plans-to-move-to-weblate-for-translation/3415) — KDE's 2023 answer.
- [Archived] [GNOME Translation Project](https://wiki.gnome.org/TranslationProject/ContributeTranslations) — historical roles and fuzzy-string status.
