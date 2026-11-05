> **Read when:** you need what KET carries for S-groups — types, nesting, component order, the field and the display
> of a data S-group — or why a KET document with S-groups fails to load.
> **Skip when:** the question is the molfile side of S-groups ([molfile-data-sgroups.md](molfile-data-sgroups.md)),
> or how the reader is built.

<!-- STUB: covers the S-group types, the nesting and the data S-groups. Bracket types and positions, and the polymer
types MON, MER, CRO, MOD, GRA and ANY are not described yet. -->

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
- WHEN an S-group has a type in the wrong case or one KET does not carry, an atom or bond the node does not have, an
  atom listed twice, an empty `atoms` while it is not a data S-group, no `mul` or one outside 1–1000 on a multiple
  group, an `id` used twice or below 0, a `parentId` naming no group, a cycle of parents, `componentOrder` on another
  type or below 1, `id` or `parentId` on a query component, a `connectivity` or `subtype` that is not a string, or a
  repeat pattern or copolymer subtype the `SGroup` tables do not name THEN the document fails to load, and the message
  names the group by its position in `sgroups` (from 0), its `id` and the molecule node.
- WHEN `connectivity` or `subtype` is given to a type that does not take it THEN it is dropped, like any key outside
  the contract.
- WHEN a molfile lists an atom of a group twice THEN KET gets it once, so every KET Indigo writes loads back.

Data S-groups (since Indigo 1.50):

- WHEN a data S-group is saved with `ket-saving-version` 2.1.0 or later THEN it carries `dataFieldDescription`
  (`fieldName`, `fieldValueType` `FORMATTED` / `NUMERIC` / `TEXT`, `fieldUnits`, `queryType` `DATA` / `QUERY`,
  `queryOperator`) and `dataDisplayInformation` (`position`, `display` `ATTACHED` / `DETACHED`, `placement`
  `ABSOLUTE` / `RELATIVE`, `displayUnits`, `displayedChars`, `fieldTag`, `daspPosition`), every field with a default
  written, and next to them the keys of KET 2.0 with the same values, deprecated until KET 3.0. Saved as 2.0 it
  carries the 2.0 keys only.
- WHEN a document has both an object and the 2.0 keys THEN the object is read and the keys are neither read nor
  checked; a field missing from an object reads as its default; a value outside its set — a type, a display, a
  placement, a `daspPosition` outside 1–9, a tag of more than one character, a field of the wrong JSON type, a
  position single precision cannot keep to four decimals (1234.5678) — fails the load with the object, the field
  and the value. A `queryOperator` outside the nine names (`EQUAL`, `NOT_EQUAL`,
  `LESS`, `LESS_OR_EQUAL`, `GREATER`, `GREATER_OR_EQUAL`, `BETWEEN`, `CONTAINS`, `LIKE`) is kept as the text it is,
  and written whenever it is set, with or without `QUERY`.
- WHEN the 2.0 keys are read THEN `placement: false` is a relative position and `true` or no key an absolute one,
  `display: true` is an attached label, a relative position without `x` and `y` is (0, 0). A relative position is
  the offset from the centre of the group's atoms — of the nearest ancestor with atoms for a group without its own;
  a molfile counts from the first atom, and the conversion happens on the way in and out.
- WHEN a data S-group is written as 2.0 THEN `fieldName`, `x` and `y` are always there, `display: true` on an attached
  label and `placement: false` on a relative one; `dataDetached` is read but no longer written.
- WHEN a data S-group has `context` (`Fragment`, `Atom`, `Bond`, `Group`, `Multifragment`) THEN it is kept through
  KET, CMF and a copy of the structure.
- WHEN a KET value lies outside what a molfile holds — an operator outside the nine, a field name over 30 characters,
  a display coordinate beyond ten columns — THEN the KET loads; writing the molfile fails
  ([molfile-data-sgroups.md](molfile-data-sgroups.md)).

## Guarantees

- MOL → KET → MOL keeps repeating units, copolymers, components, mixtures and formulations with their subtypes, repeat
  patterns, component order and nesting. KET carries no bracket coordinates: a bracketed group (SRU, COP, COM, MIX,
  FOR) gets its brackets anew when the KET is loaded, nested inside those of its parent
  ([sgroup-brackets.md](sgroup-brackets.md)); the brackets a MOL file had are not kept.
- `id` is a reference inside one molecule node, not an identifier: a reader may renumber the groups once the
  references are resolved.
- The atoms of an S-group are saved in one molecule node, also when a monomer joins them.

## Limitations

- A molfile in which one group lies inside another without `M  SPL` / `PARENT` gains `PARENT` after a round trip
  through KET.
- Ketcher writes no `parentId`: after Ketcher edits a document, a data S-group without atoms and groups with equal
  atom sets lose their parent.
- `MON`, `MER`, `CRO`, `MOD`, `GRA` and `ANY` are refused by the reader and the writer.
- A data S-group with neither atoms nor a parent is lost when the structure is copied, saved to KET or read from KET;
  a molfile keeps it.
- KET 2.0 has no place for "show the units": the flag is lost when a structure is saved as 2.0; the
  `dataDisplayInformation` object of 2.1 keeps it as `displayUnits`. Indigo before 1.50 wrote the flag under
  `display`, which means "attached" by the contract.
- Indigo before 1.50 reads `placement: false` as an absolute position, and compares the document version as a string,
  so it drops the text nodes of a document whose `ket_version` is 2.1.0.
