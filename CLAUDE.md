@AGENTS.md

## Claude Code specifics

The instructions above are vendor-neutral and live in [AGENTS.md](AGENTS.md) so that every assistant
reads the same file. This section is the part that only Claude Code can act on.

- **`.claude/rules/` load themselves.** Each rule declares the paths it governs, so the C++, Python,
  Java, CMake, CI and integration-harness conventions enter the context only when a matching file is
  read. Do not restate them here, and do not read them all up front.
- **`.claude/skills/` are procedures for tests.** `/indigo-testing` what a change must pass and how to sort a red run,
  `/indigo-add-tests` what a new test must cover, `/indigo-reference-tests` the order of operations
  for a reference test in the integration harness.
- **`/memory-bank`** folds finished work back into `.memory-bank/`, or answers a question from it and
  closes the gap when the bank cannot. Run it when a change taught something lasting.
- **`.claude/scripts/check-anchors.sh`** verifies every `` `<path>#<Symbol>` `` anchor in the
  documentation. Run it after editing the bank; a broken anchor is a defect in the document.
