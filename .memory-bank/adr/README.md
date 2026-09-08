# Architecture Decision Records

> **Read when:** you are about to reverse, extend, or re-argue a structural decision.
> **Skip when:** you are implementing inside an existing structure.

One file per decision, named `<slug>.md` — the date belongs in the header, not in the file name.
Records are immutable: a decision that is later reversed gets a **new** record that supersedes the
old one, and the old one gains a `Superseded by:` line. Editing history away destroys the only
reason to keep it.

## Format

```markdown
# <Decision in one line>

**Date:** YYYY-MM-DD · **Status:** Accepted | Superseded by <file> · **Ticket:** #NNNN

## Context
What forced a choice. Constraints, measurements, the state of the code at the time.

## Decision
What was chosen, stated so that a reader can check whether the code still complies.

## Alternatives considered
Each with the reason it lost. An ADR with no rejected alternative is a note, not a decision record.

## Consequences
What this makes easy, what it makes hard, and what must now be maintained forever.
```

## When to write one

Write a record when a change sets a precedent others will follow or fight: a new container or
ownership model, a new layer or boundary, a dependency added or dropped, an option/contract that
becomes public, a deliberate deviation from a format specification.

Do **not** write one for a bug fix, a refactor with no interface change, or a decision already
recorded in a ticket that the code links to.

## Records

- [owning-containers.md](./owning-containers.md) — objects are owned by
  `PtrArray` and `PtrReusablePool`, not by `ObjArray` or `std::vector<T>`; the `Reusable` contract
- [haptic-bond-endpoints.md](./haptic-bond-endpoints.md) — an attachment group is stored beside the
  graph, never as a vertex, so graph algorithms stay correct on organometallics by construction

Still worth reconstructing from the analyses in `indigo-common/Task/`: the valence-model
configuration contract (BIOVIA 2009 vs 2017), and the decision to route non-local formats through
Indigo rather than duplicating parsers in Ketcher.
