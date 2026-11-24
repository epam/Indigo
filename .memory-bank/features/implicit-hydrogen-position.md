# Where the implicit hydrogens of an atom label stand

> **Read when:** a ticket is about the side of an atom symbol its `H`, `H2`, `H3` is drawn on, about a radical
> dot or a charge that stands in the wrong place next to them, or about a bracket or a bond that runs into them.
> **Skip when:** the question is how many hydrogens an atom has: that is
> [valence-and-implicit-hydrogens.md](./valence-and-implicit-hydrogens.md).

Verified on 2026-11-24. Since Indigo 1.51.

## Problem

Indigo chose the side of the hydrogens in two places that disagreed. The picture weighed four sides and then
moved the hydrogens away from neighbouring atoms; the estimate of a label's size, which places the brackets of an
S-group, knew left and right only. Brackets were therefore placed around hydrogens that were drawn elsewhere.
Ticket #4031 describes one rule, the same for Indigo and for the Ketcher canvas.

## Interface

Nothing to call and no option: every rendering of a molecule or a reaction, and every placement of S-group
brackets. Angles below are as drawn, 0° to the right and 90° up, and are compared in hundredths of a degree.

## Expected behaviour

**When** an atom has one bond, **then** its hydrogens stand on the left when the bond lies in the right half and
on the right otherwise. Straight up belongs to the right half and straight down to the left one: an oxygen below
its carbon is `HO`, a methyl above its neighbour is `CH3`. There is no tolerance: a bond at 89.99° gives the
left, a bond at 90.01° the right.

**When** an atom has no bonds, **then** its hydrogens stand where its formula is written: on the left for O, F,
S, Cl, Se, Br and I (`H2O`, `HCl`, `H2S`), on the right for every other element (`NH3`, `CH4`).

**When** an atom has two or more bonds, **then** its hydrogens stand in the first quadrant without a bond, in the
order right, left, top, bottom. A quadrant reaches 45° to both sides of its axis and includes its
counter-clockwise end: a bond at 45° is in the right quadrant, at 135° in the top one, at 225° in the left one,
at 315° in the bottom one.

**When** the quadrant opposite to that one is free as well, **then** the hydrogens stand in the one the bonds
point away from, and in the first of the two when the bonds balance. A carbon at the bottom of a zigzag has its
hydrogens below and one at the top above; the left nitrogen of a piperazine ring is `HN`, with its hydrogen
outside the ring.

**When** no quadrant is free, **then** the hydrogens stand on the right.

**When** an atom has a radical, **then** the radical takes the top quadrant of an atom with two or more bonds,
whatever its kind. A doublet is drawn above the symbol. When a bond of the atom points up, within 24° of the
vertical, it is drawn at the upper right of the symbol instead, as before Indigo 1.51, unless hydrogens, a
charge or a valence stand there. A singlet and a triplet are always drawn above the symbol.

**When** hydrogens stand above or below the symbol, **then** `H` is centred on it with the count to the right of
`H`, and a bond that reaches them is cut short there, as at any part of a label; so is the line of an attachment
point. A charge and a valence start to the right of a count above the symbol; with the hydrogens below they stay
at the symbol.

**When** the brackets of an S-group are placed, **then** the box of an atom label includes its hydrogens where
they are drawn: above or below the symbol it is taller, beside it wider.

## Guarantees

- The side depends on the bonds of the atom, on its radical and, for an atom without bonds, on its element: not
  on neighbouring atoms, not on the font, not on the label mode.
- The picture and the label box used for brackets take the side from the same rule.
- Which atoms show hydrogens at all is unchanged, and so is the alignment of a pseudo-atom or an atom list.
- An atom without bonds has its hydrogens where they stood before Indigo 1.51.

## Limitations

- The rule departs from the text of #4031 in four places. With top and bottom both free the hydrogens go away
  from the bonds, where requirement 2.2 gives the top: this is what the ticket's own hexane picture shows. The
  same holds for right and left, where 2.2 gives the right. An atom without bonds follows its element, where
  requirement 1.1 gives the right. A radical leaves the top when a bond is there, where the ticket places every
  radical in the top quadrant.
- A bond a hundredth of a degree off the vertical already decides the side, so two drawings that look alike can
  differ in `H3C` and `CH3`.
- Hydrogens are not moved away from a neighbouring atom, so in a crowded drawing they can meet its label or its
  bracket.
- A bond on the side where the count of hydrogens above or below the symbol stands is cut short by them, also
  when it passes clear of the count: a `CH2` on a straight line loses about a sixth of the bond length on its
  right from 10° upwards. At 35° to 45° the bond reads as if it left the hydrogens.
- A bracket of an S-group stands close to a vertical bond whose lower atom is written `H3C`: the label no longer
  widens the unit on that side.
- When a bond points up and the place at the right holds hydrogens, a charge or a valence, the dot of a radical
  stands above the symbol, on the axis of the bond. A dot at the upper right can stand next to a bond that
  leans right, and the second line of a double or a triple bond can point at a dot above the symbol.
- A reaction layout measures labels only when a font size is given (`render-font-size`); without it the side of
  the hydrogens does not change where molecules stand.
- Ketcher implements the same ticket separately (epam/ketcher#12564, open when this was written). Where its
  reading differs, as for a vertical bond and for a bond on a diagonal, a picture differs from the canvas.
