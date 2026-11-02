# S-group brackets are placed by the layout, two vertical brackets at fixed margins around the group's bounding box

**Date:** 2026-10-22, revised 2026-10-27 · **Status:** Accepted · **Ticket:** #3974

## Context

Until this change the layout placed brackets for repeating units only: one bond length long, across the chain
at the centres of its atoms, a bracket per crossing bond. Copolymers, components, mixtures and formulations
kept whatever a file had, KET structures had zero-length placeholders, and the renderer drew brackets of its
own for structures without any, so a structure looked different before and after `layout()`. The polymer
work for the customer (#3971) needs brackets that show membership and nest, written to MOL for Ketcher and
BIOVIA Draw to read.

## Decision

Brackets are a property of the layout, computed in the core by `SGroupBracketLayout` and `SGroupNesting`,
never by the renderer or by a loader directly. Every bracketed group gets exactly two vertical brackets at
fixed margins: 0.1 bond lengths left and right of the box of its atom labels and 0.3 above and below it, and
0.3 outside the brackets of the groups nested in it, whichever reaches further. The unit is the default bond
length constant, the label boxes come from the existing `getAtomBoundingBox` estimate at the font of the
layout options, and brackets read from a file are kept while their atoms stay. Nothing outside the group is
consulted.

## Alternatives considered

- Brackets through the midpoints of the crossing bonds, as BIOVIA draws them and as Ketcher's own V2000
  writer places them (the first step of the ticket, commit `784b0ac61`): rejected by the Tech Lead review of
  2026-10-17, because a bracket ran through a label, a side group stood outside the brackets, and brackets
  came out horizontal or slanted.
- Sharing the gap with the nearest atom outside the group: a step from 0.3 down to 0.15, the brackets of a
  parent kept clear of the children's labels and repeat patterns, brackets at least one bond length long, the
  unit the mean bond length of the drawing (the second version, branch `3974-sgroup-brackets-gap-sharing`,
  tag `3974-sgroup-brackets-gap-sharing-v1`). Replaced on 2026-10-27 when the product owner rewrote the
  ticket to the fixed margins for simplicity: a bracket through a neighbouring atom was accepted as the
  price.
- Placement in the renderer: it would not reach the MOL file, which is what Ketcher and BIOVIA read.
- Measuring labels with the renderer's fonts: needs cairo in the core; the core's estimate was accepted,
  with the exact font metrics left for later.

## Consequences

- Every code path that moves atoms must move or re-place brackets (invariant A13); the order of the two
  brackets is a contract with the renderer (A12); zero-length brackets are placeholders (A14).
- A structure drawn at another bond length gets margins out of proportion, and a bracket can cross a
  neighbouring atom or the label of a nested group.
- Ketcher's references with `BRKXYZ` (about 586 files) change when this lands.
- The layout of reactions (#3981) and the renderer's handling of all five types (#3976) stay separate tickets.
