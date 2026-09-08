# GitHub Copilot instructions

The instructions for this repository are in [AGENTS.md](../AGENTS.md), which is written for any AI
assistant rather than for one vendor. Read it first; it routes to the knowledge base in
[.memory-bank/](../.memory-bank/), which is read one file at a time.

Before changing anything under `core/`, `api/c/`, `api/cpp/` or `bingo/`, read
[.memory-bank/invariants.md](../.memory-bank/invariants.md): every rule there fails silently.

Per-language conventions are in [.claude/rules/](../.claude/rules/) — one file per area, each
naming the paths it governs. Read the one matching the code you are about to touch.

This file deliberately holds no rules of its own. A second copy of the conventions drifts from the
first, and a stale instruction is followed just as confidently as a current one.
