# git commit

> Commit staged files to the repository.

- Stage all modified tracked files and commit with a message:

`git commit {{[-a|--all]}} {{[-m|--message]}} "{{commit_message}}"`

- Reference a nested brace expression and escaped braces:

`git commit -m "{{stash@{0}}}" \{\{literal\}\}`
