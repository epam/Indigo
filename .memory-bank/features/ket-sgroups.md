> **Read when:** you need what KET carries for S-groups — types, nesting, component order — or why a KET document
> with S-groups fails to load.
> **Skip when:** the question is the molfile side of S-groups, or how the reader is built.

<!-- STUB: covers the S-group types and the nesting. Data S-group keys (kind of value, units, context, position),
bracket types and positions, and the polymer types MON, MER, CRO, MOD, GRA and ANY are not described yet. -->

# S-groups in KET

## Problem

KET is the format Ketcher exchanges with Indigo. Polymers, mixtures and formulations drawn for registration reach
MOL through it, so every S-group type, the component order and the nesting of S-groups have to survive
KET ⇄ MOL.

## Interface

Loading a KET document as a molecule or a query, and saving a structure to KET, through any wrapper, the WASM module
or the REST service.

## Expected behaviour

- WHEN an S-group has type `COM`, `MIX` or `FOR` THEN it is read and written; a component keeps its order under
  `componentOrder`.
- WHEN an S-group has `parentId` THEN its parent is the S-group of the same molecule node whose `id` it is.
- WHEN no S-group of the whole document — molecule nodes, R-group fragments — has `parentId`, as in KET from Ketcher,
  THEN the parent of a group is the smallest group of its node whose atoms strictly contain its atoms, the first of
  two equal ones. A data S-group is nobody's parent; a group without atoms and a group whose atoms equal another's
  get none.
- WHEN a structure with nested S-groups is saved THEN every nested group gets `parentId` and every group referred to
  gets `id`; a structure without nesting is saved without either key, as before Indigo 1.50. A parent and a child without a
  common atom are saved in one molecule node.
- WHEN an S-group has a type in the wrong case or one KET does not carry, an atom or bond the node does not have,
  `mul` outside 1–1000, an `id` used twice or below 0, a `parentId` naming no group, a cycle of parents,
  `componentOrder` on another type or below 1, `id` or `parentId` on a query component, or a `connectivity` or
  `subtype` that is not a string THEN the document fails to load, and the message names the group by its position in
  `sgroups` (from 0), its `id` and the molecule node.
- WHEN `connectivity` or `subtype` has a value outside the contract THEN it reads as either/unknown and as no
  subtype, as before #3954.

## Guarantees

- MOL → KET → MOL keeps repeating units, copolymers, components, mixtures and formulations with their subtypes, repeat
  patterns, component order and nesting. Brackets are not kept: KET has no place for them yet.
- `id` is a reference inside one molecule node, not an identifier: a reader may renumber the groups once the
  references are resolved.

## Limitations

- A molfile in which one group lies inside another without `M  SPL` / `PARENT` gains `PARENT` after a round trip
  through KET.
- Ketcher writes no `parentId`: after Ketcher edits a document, a data S-group without atoms and groups with equal
  atom sets lose their parent.
- `MON`, `MER`, `CRO`, `MOD`, `GRA` and `ANY` are refused by the reader and the writer.
- A data S-group with neither atoms nor a parent is lost when the structure is copied, saved to KET or read from KET;
  a molfile keeps it.
- An S-group over atoms that a monomer joins is saved to KET once per molecule node on each side of the monomer.
