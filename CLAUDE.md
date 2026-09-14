@AGENTS.md

## Claude Code specifics

The instructions above are vendor-neutral and live in [AGENTS.md](AGENTS.md) so that every assistant
reads the same file. This section is the part that only Claude Code can act on.

- **`.claude/rules/` load themselves.** Each rule declares the paths it governs, so the C++, Python,
  Java, CMake and CI conventions enter the context only when a matching file is read. Do not restate
  them here, and do not read them all up front.
- **`/memory-bank`** folds finished work back into `.memory-bank/`, or answers a question from it and
  closes the gap when the bank cannot. Run it when a change taught something lasting.
- **`.claude/scripts/check-anchors.sh`** verifies every `` `<path>#<Symbol>` `` anchor in the
  documentation. Run it after editing the bank; a broken anchor is a defect in the document.
