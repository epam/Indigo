# An attachment group is not a graph vertex

**Date:** 2026-09-02 · **Status:** Accepted · **Tickets:** #3233 family

<!-- STUB: reconstructed from the code and from indigo-common/Task/3233/. The deliberation is in
     that folder; this record exists because the code now depends on the decision. -->

## Context

A haptic bond connects a metal to a *set* of ligand atoms acting as one end. The set has to be
addressable — a bond endpoint must be able to name it — which makes it look like a node in the
molecule.

The molecular graph is walked by nearly everything in the core: valence, aromaticity perception,
canonical ranking and canonical SMILES, InChI, fingerprints, substructure matching, layout,
rendering. All of them iterate with `vertexBegin()` / `vertexNext()` and assume every vertex is an
atom.

## Decision

The attachment group is stored **beside** the graph, not in it. `MoleculeAttachmentGroups` and
`MoleculeHapticBonds` are members of `BaseMolecule` (`base_molecule.h:572`, `:574`); a group is
never a vertex and never appears in vertex iteration. A `HapticBond::Endpoint` is a tagged value —
either an atom index or a group index.

Two consequences are part of the decision, not accidents of it:

- **A group stores no position.** Anything needing one derives it from the member atoms.
- **Creation and removal are funnelled.** `addHapticBond` is the only way to make a haptic bond, and
  `removeAttachmentGroup` the only way to remove a group, so the invariant "no haptic bond has a
  dangling endpoint" is maintained in one place rather than at every call site.

## Alternatives considered

- **Make the group a vertex of the graph.** Rejected: every algorithm walking `vertices()` would
  have to learn to skip it, and each one that forgot would fail silently on organometallics. The
  cost is paid by code that has nothing to do with the feature.
- **Represent the haptic bond as N ordinary bonds, one per member atom.** Rejected: it states
  something chemically different, changes computed valence and connectivity, and cannot round-trip
  back into a single haptic bond.
- **Give the group its own coordinates.** Rejected: a second source of truth for a position that is
  fully determined by the member atoms drifts the moment the ligand moves.

## Consequences

- Graph algorithms are correct on organometallics **by construction** — they never see the group.
- The price is paid at the lifecycle points instead: attachment groups and haptic bonds need their
  own handling in `clear()`, in merge, in `removeAtoms`, and their own copy flags
  (`SKIP_ATTACHMENT_GROUPS`, `SKIP_HAPTIC_BONDS`). That is the general rule for molecule side data
  ([../invariants.md](../invariants.md), A4 and A5), and this feature is the largest instance of it.
- Formats that have no concept of a collective endpoint cannot represent the structure faithfully;
  each such format needs an explicit decision about what to write, rather than a silent fallback.
- Anything that needs a point for the group — rendering, and layout when it gains support —
  computes the centroid from the member atoms. That computation belongs in **one** function shared
  by its callers; duplicating it is how two parts of the UI end up disagreeing about where the bond
  points.
