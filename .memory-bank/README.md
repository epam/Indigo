# Memory Bank — conventions

> **Read when:** you are about to write into `.memory-bank/`, or you need the file-format rules.
> **Skip when:** you only need to *read* knowledge — go straight to the file named in [../CLAUDE.md](../CLAUDE.md).

The memory bank is the canonical, code-adjacent knowledge base for this repository. It is committed
with the code so that it can be reviewed, corrected, and kept honest by the same process as the code.

## Layout

```
.memory-bank/
├── README.md         # this file — conventions only
├── architecture.md   # how the repo is organised: components, layers, data flow
├── domain.md         # cheminformatics concepts as this codebase models them
├── glossary.md       # term → definition → where it is used
├── invariants.md     # rules that must never break, with IDs and verification anchors
├── build.md          # configure/build/targets/WASM/devcontainer
├── testing.md        # how to run every suite
├── conventions.md    # code style and tooling
├── modules/          # one deep-dive per subsystem   (README.md = index + format)
├── features/         # observable behaviour per feature (README.md = index + format)
└── adr/              # architecture decision records  (README.md = format)
```

## Every file starts with a read budget

The first two lines of every memory-bank file are:

```markdown
> **Read when:** <the situations in which this file pays for its tokens>
> **Skip when:** <the situations in which it does not>
```

This exists so an agent can decide **not** to open a file. A knowledge base that must be read in full
before it is useful is a tax on every session; a routing table plus honest per-file scope is not.

## Document formats

| File               | Answers                                                                              |
| ------------------ | ------------------------------------------------------------------------------------ |
| `architecture.md`  | Component structure, layering, data flow, where a change of kind X belongs           |
| `domain.md`        | Entities, relationships, constraints — chemistry as modelled here, not as in textbooks |
| `glossary.md`      | Term, definition, where it is used in code                                            |
| `invariants.md`    | ID, statement, why it breaks silently, anchor (`file:line`)                          |
| `modules/<x>.md`   | Responsibility, public interface, dependencies, dependents, constraints & traps      |
| `features/<x>.md`  | Problem, user interaction, expected behaviour (WHEN/THEN), guarantees, limitations    |
| `adr/<date>-<x>.md`| Decision, context, alternatives considered, rationale, consequences                  |

## Rules for writing

- **State facts, anchored.** Every non-obvious claim carries a `path/file.cpp:NNN` anchor and, where
  the claim is about behaviour rather than structure, the commit it was verified on. An anchor that
  no longer resolves is a bug report against this file.
- **Non-obvious only.** If a competent reader learns it in thirty seconds from the source, it does
  not belong here. Directory listings, class inventories, and restated signatures are not knowledge.
- **Traps outrank descriptions.** The reason this base exists is the silent failure — the second edit
  a change requires, the branch that has a parallel twin, the cache that is not invalidated.
- **Mark incompleteness.** A file that covers part of its subject opens with `<!-- STUB: … -->`
  naming what is missing. Silent partial coverage is worse than an admitted gap.
- **No implementation detail in `features/` or `domain.md`.** Function names, paths and variables
  belong in `modules/` or in the code.

## When to update

- **During a change** — if you find something here wrong or missing, fix it in the same commit.
- **After a change lands** — extract the lasting knowledge: new trap → `invariants.md` or the
  relevant `modules/` file; new behaviour → `features/`; structural decision → `adr/`.
- **Never** as a separate "documentation pass" months later. Knowledge written from memory instead
  of from the diff is how this kind of base rots.
