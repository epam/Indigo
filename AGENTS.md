# Repository instructions

## Git safety

- Use Git only for read-only inspection, such as `status`, `diff`, `log`, and `show`.
- Never run Git commands that change the working tree, index, refs, or remotes. This includes `add`, `commit`, `push`, `pull`, `fetch`, `restore`, `checkout`, `switch`, `reset`, `merge`, `rebase`, `stash`, `tag`, `branch`, and `clean`.
- Never modify files inside `.git`. Ask the user to perform any required Git write operation.
