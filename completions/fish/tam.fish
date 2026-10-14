# Fish completion for tam(1)
# Static completion: subcommands and fixed flags only. Zero file I/O.

# Global flags
complete -c tam -f -s h -l help -d "Show usage information"
complete -c tam -f -s V -l version -d "Show version information"

# Subcommands
set -l subcommands show welcome explain search man install help
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a show -d "Open offline guide in pager or text"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a welcome -d "First-run orientation"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a explain -d "Curated topic explainer"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a search -d "Search corpus index"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a man -d "Native manual page viewer"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a install -d "Installation inspection and framework"
complete -c tam -f -n "not __fish_seen_subcommand_from $subcommands" -a help -d "Display usage help"

# Subcommand flags
complete -c tam -f -n "__fish_seen_subcommand_from show welcome explain" -l pipe -d "Emit plain text without terminal controls"
complete -c tam -f -n "__fish_seen_subcommand_from search" -l json -d "Emit machine-readable JSON output"
complete -c tam -f -n "__fish_seen_subcommand_from search" -l pipe -d "Emit plain text without terminal controls"
complete -c tam -f -n "__fish_seen_subcommand_from man" -l help -d "Show man usage"
complete -c tam -f -n "__fish_seen_subcommand_from man" -l pipe -d "Emit plain text without terminal controls"

# install subcommands
complete -c tam -f -n "__fish_seen_subcommand_from install; and not __fish_seen_subcommand_from inspect plan apply verify" -a "inspect plan apply verify --help"
complete -c tam -f -n "__fish_seen_subcommand_from install; and __fish_seen_subcommand_from inspect" -l pipe -d "Emit plain text without terminal controls"
