# S-group brackets are placed by the layout, two vertical brackets around the group's bounding box

**Date:** 2026-10-22 · **Status:** Accepted · **Ticket:** #3974

## Context

Until this change the layout placed brackets for repeating units only: one bond length long, across the chain
at the centres of its atoms, a bracket per crossing bond. Copolymers, components, mixtures and formulations
kept whatever a file had, KET structures had zero-length placeholders, and the renderer drew brackets of its
own for structures without any, so a structure looked different before and after `layout()`. The polymer
work for the customer (#3971) needs brackets that show membership and nest, written to MOL for Ketcher and
BIOVIA Draw to read.

## Decision

Brackets are a property of the layout, computed in the core by `SGroupBracketLayout` and its helpers
(`SGroupNesting`, `AtomBoxes`, `SGroupTexts`, `BracketSpacing`), never by the renderer or by a loader
directly. Every bracketed group gets exactly two vertical brackets around the bounding box of its atom labels
and nested brackets, with the distances of the ticket's table; brackets read from a file are kept while their
atoms stay. The classes are split by responsibility and use no inheritance: one spacing rule exists, and a
second one would introduce the interface.

## Alternatives considered

- Brackets through the midpoints of the crossing bonds, as BIOVIA draws them and as Ketcher's own V2000
  writer places them (the first step of the ticket, commit `784b0ac61`): rejected by the Tech Lead review of
  2026-10-17, because a bracket ran through a label, a side group stood outside the brackets, and brackets
  came out horizontal or slanted.
- Placement in the renderer: it would not reach the MOL file, which is what Ketcher and BIOVIA read.
- Measuring labels with the renderer's fonts: needs cairo in the core; the core's estimate was accepted,
  with the exact font metrics left for later.
- The median bond length as the unit: rejected, the reference planner and the rest of the library use the
  mean.

## Consequences

- Every code path that moves atoms must move or re-place brackets (invariant A13); the order of the two
  brackets is a contract with the renderer (A12); zero-length brackets are placeholders (A14).
- Ketcher's references with `BRKXYZ` (about 586 files) change when this lands.
- The layout of reactions (#3981) and the renderer's handling of all five types (#3976) stay separate tickets.
