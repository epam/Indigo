---
description: Update .memory-bank/ from work just completed, or answer a question from it
argument-hint: "[topic or 'sync'] — e.g. 'sync', 'valence model', 'bingo oracle install'"
allowed-tools: Read, Glob, Grep, Edit, Write, Bash(git diff:*), Bash(git log:*), Bash(git show:*)
---

Target: **$ARGUMENTS**

## Mode A — `sync` (or no argument): fold finished work back into the bank

1. Read [.memory-bank/README.md](../../.memory-bank/README.md) for the file formats and writing rules.
2. Read the actual change: `git diff` for uncommitted work, otherwise `git log -1 -p`. Base every
   claim on the diff, never on recollection of the conversation.
3. Decide what, if anything, is *lasting* knowledge. Most changes teach nothing durable — say so and
   stop. Only these earn an edit:
   - a failure mode that is silent (no compile error, no failing test locally) → `invariants.md`,
     with an ID, the reason it fails quietly, and a `path/file.cpp:NNN` anchor
   - subsystem behaviour that cost you real reading time → `modules/<name>.md`
   - a change in what a capability promises callers → `features/<name>.md`
   - a structural decision that sets precedent → `adr/<slug>.md`
   - a term you had to look up → `glossary.md`
4. Check the anchors you are about to touch still resolve; a stale `file:line` in the bank is a
   defect, fix it while you are there.
5. Report each edit as one line: `file — what changed and why`. If nothing qualified, report that.

## Mode B — a topic: answer from the bank, then close the gap

1. Route through [CLAUDE.md](../../CLAUDE.md) to the one file that should hold the answer; read only
   that file, plus what it links to.
2. If it answers the question — report the answer with the file it came from, and stop.
3. If it does not, find the answer in the source, report it, and then write it into the bank in the
   right place, marking the file's `<!-- STUB -->` line if coverage is still partial.

## Rules

- Never restate what the source already makes obvious in thirty seconds.
- Never widen scope: this command edits `.memory-bank/`, not code.
- Prefer correcting an existing file to creating a new one; duplicated knowledge diverges.
