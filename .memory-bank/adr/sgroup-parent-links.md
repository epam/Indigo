# An S-group parent is a pool position; the index is a fallback, resolved by one rule

**Date:** 2026-10-22 · **Status:** Accepted · **Since:** Indigo 1.50

## Context

An S-group names its parent by its pool position (`parent_idx`), by the parent's `index` (`parent_group`), or — in
the CML reader — by a one-based pool position stored in `parent_group`. Three places resolved this,
each its own way: the savers' ordering, a recursive lookup used when copying S-groups, and the rendering tree, which
read `parent_idx` alone. The copy remapped `parent_idx` only when the target held fewer S-groups than the source, so
`merge()` into a molecule with S-groups left stale positions behind — a group became its own parent and the next copy
overflowed the stack — and a MOL with a parent on another fragment could not be saved to KET. KET nesting needs
the same copy to be right: its reader joins the molecule nodes of a document through it.

## Decision

1. `MoleculeSGroups::resolveParents` is the only resolver: `parent_idx`, else the group whose `index` is
   `parent_group`, else the group at the one-based position `parent_group` when that group has no `index`. The
   savers, the copy of S-groups and the rendering tree go through it.
2. A copy resolves the parents in the source and remaps them for every copied group; a parent left behind leaves the
   copy without a parent, both links cleared.
3. `MoleculeSGroups::remove` clears `parent_idx` of the children of the removed group. `BaseMolecule::removeSGroup`
   first moves them to the grandparent, by `parent_group` only when the removed group has an `index`.

## Alternatives considered

- **Renumber `index` after every copy and keep both links in step.** A second numbering that no requirement asks
  for, and every reader would still have to choose between the two links.
- **Keep the recursive lookup for copies.** A second rule: it fell back to a lookup by `index`, which for groups
  numbered 0 finds the first group of the pool — the child itself when it comes first.
- **Move children to the grandparent inside `MoleculeSGroups::remove`.** About a dozen places remove S-groups directly,
  savers among them; their output would change for every structure whose removed group had children.

## Consequences

- New code that needs a parent calls `resolveParents`, not the fields; it costs one pass over the groups and one
  hash map, so it is called once per operation, not once per group.
- `parent_idx` is the link the code can rely on; `parent_group` stays for the formats that store numbers.
