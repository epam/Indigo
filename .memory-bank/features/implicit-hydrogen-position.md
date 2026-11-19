# Where the implicit hydrogens of an atom label stand

> **Read when:** a ticket is about the side of an atom symbol its `H`, `H2`, `H3` is drawn on, about a picture
> that differs from the Ketcher canvas there, or about a bracket or a reaction arrow that runs into hydrogens.
> **Skip when:** the question is how many hydrogens an atom has: that is
> [valence-and-implicit-hydrogens.md](./valence-and-implicit-hydrogens.md).

Verified on 2026-11-19. Since Indigo 1.51.

## Problem

Indigo and Ketcher each chose the side of the hydrogens by a rule of their own, so a PNG or an SVG showed a
structure differently from the canvas it was drawn on. Inside Indigo the picture and the estimate of a label's
size disagreed as well: the picture could put the hydrogens above or below the symbol, the estimate knew left
and right only, and the brackets of an S-group were placed by the estimate.

## Interface

Nothing to call and no option: every rendering of a molecule or a reaction, and every placement of S-group
brackets. Angles below are as drawn, 0° to the right and 90° up, and are rounded to 0.01°.

## Expected behaviour

**When** an atom has one bond, **then** its hydrogens stand opposite the bond: on the left when the bond points
right, on the right when it points left. A bond within 0.5° of vertical, up or down, gives the right.

**When** an atom has no bonds, **then** its hydrogens stand on the right, for every element: `OH2`, `ClH`,
`NH3`.

**When** an atom has two or more bonds, **then** its hydrogens stand in the first quadrant without a bond, in the
order right, left, top, bottom. The quadrants are right [315°, 45°), top [45°, 135°), left [135°, 225°) and
bottom [225°, 315°); a bond on a boundary belongs to the quadrant that starts there.

**When** that quadrant is the top one and the bottom one is free as well, **then** the hydrogens go away from
the bonds: below the symbol when the bonds point up on balance, above it when they point down or are level. A
nitrogen at the top of a zigzag has its hydrogen above, one at the bottom has it below.

**When** no quadrant is free, **then** the hydrogens stand on the right.

**When** an atom has a radical, **then** the radical is drawn above the symbol, whatever its kind, and takes the
top quadrant of an atom with two or more bonds. An atom with one bond or none keeps its hydrogens beside the
symbol.

**When** hydrogens stand above or below the symbol, **then** `H` is centred on it with the count to the right of
`H`; they do not shorten a bond; a charge and a valence start to the right of the count, not on top of it.

**When** the brackets of an S-group are placed, **then** the box of an atom label includes its hydrogens where
they are drawn: above or below the symbol it is taller, beside it wider.

## Guarantees

- The side depends on the bonds and the radical of the atom alone: not on neighbouring atoms, not on the font,
  not on the label mode.
- The rule and its numbers are those of Ketcher's implementation of the same ticket, so a structure looks the
  same in a picture and on the canvas.
- The picture and the label box used for brackets take the side from the same rule.
- Which atoms show hydrogens at all is unchanged, and so is the alignment of a pseudo-atom or an atom list.

## Limitations

- An atom with both bonds on its right, such as the left nitrogen of a piperazine ring drawn with flat top and
  bottom, has its hydrogen on the right, inside the ring: the right quadrant is free and comes first.
- Hydrogens are no longer moved away from a neighbouring atom, so in a crowded drawing they can overlap its label.
- A structure drawn before this change keeps its coordinates: `H2O` next to a reaction arrow, or brackets stored
  in a file for `H2O`, now meet `OH2` on the other side.
- A reaction layout measures labels only when a font size is given (`render-font-size`); without it the side of
  the hydrogens does not change where molecules stand.
- The rule follows the open Ketcher pull request epam/ketcher#12564 (ticket epam/ketcher#12530). It departs from
  the text of the ticket in two places: a bond straight up gives the right, where requirement 1 gives the left,
  and the away-from-the-bonds case above, where requirement 2.2 gives the top. An atom without bonds has its
  hydrogens on the right as Ketcher reads requirement 1.1 (#4031).
