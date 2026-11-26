# The side of implicit hydrogens is one rule in the core, the rule of its ticket

**Date:** 2026-11-24 · **Status:** Accepted · **Since:** Indigo 1.51

## Context

Three places decided where the hydrogens of an atom label stand. The renderer weighed four sides by the
projections of the bonds and then moved the hydrogens away from neighbouring atoms. The core's estimate of a
label box, which places the brackets of S-groups and spaces a reaction, knew left and right only. Ketcher drew
the canvas by a third rule. In 18 of 82 atoms of a test set the estimate put the hydrogens where the picture did
not.

Ticket #4031 describes a common rule for both products: by the half the bond lies in for an atom with one bond,
the first free quadrant for an atom with more. Ketcher implements it in pull request epam/ketcher#12564, which
was open and unreviewed while this was done. The first version of this change repeated that implementation
number for number, and its review found three things: the copy departed from the text of the ticket for a
vertical bond and for a bond on a diagonal, an atom without bonds was written `OH2` and ran into what a stored
drawing had next to it, and hydrogens above the symbol were drawn over bonds.

## Decision

One class in the core, `ImplicitHydrogenSide`, decides the side from the directions of the atom's bonds, its
radical and, without bonds, its element. The renderer and the estimate of the label box both take the side from
it and keep no rule of their own.

The rule is the text of the ticket, read literally: the halves and the quadrants with the ends the ticket
includes, the order right, left, top, bottom, no tolerance. It departs from the text in four places, each where
the text gives a picture worse than the one Indigo drew before:

- With two opposite quadrants both free the hydrogens go away from the bonds. For top and bottom this is the
  ticket's own hexane picture, which its text contradicts; for right and left it keeps the hydrogen of a ring
  nitrogen outside the ring.
- An atom without bonds keeps the side of its element, `H2O` and `HCl`. By the definitions of the ticket such an
  atom is neither terminal nor internal.
- The dot of a radical stands above the symbol and moves to its upper right when a bond points up. The ticket
  places every radical in the top quadrant and does not say what happens when a bond is there.

Another implementation of the ticket is not a source for the rule.

## Alternatives considered

- Keep the renderer's weights and teach the estimate the same weights. Rejected: the ticket asks for one stated
  rule, and the weights with their move away from neighbours are not one.
- Repeat Ketcher's implementation number for number, as the first version did. Rejected in review: it was an
  unreviewed pull request, it departed from the ticket where the ticket is explicit, and Indigo cannot test
  against it.
- Follow the text of the ticket in every case. Rejected: the pictures named above. `OH2` next to a reaction arrow
  of a stored drawing runs into the arrow, and a hydrogen inside a six-membered ring was outside it before.
- A tolerance around the vertical for an atom with one bond. Rejected: whichever way it leans, some bond gets a
  side the text does not give, and rounding the angle to 0.01° already keeps float noise off the boundary.
- Keep the move away from neighbouring atoms. Rejected: the side then depends on more than the atom's bonds, so
  the estimate could not share the rule without searching for neighbours too.
- Move the dot of a radical to the right whenever a bond lies in the top quadrant. Rejected: the place at the
  right is itself at about 43° from the atom, so a bond at 50° sends the dot onto itself.
- Choose the place of the dot by its distance to the lines of every bond. Rejected after three rounds of
  review: each round found inputs where the choice was wrong, for double bonds and at the usual angles.
- Leave hydrogens above and below the symbol out of bond trimming. Rejected: a bond and the line of an
  attachment point are then drawn over the count.
- A method of `BaseMolecule`. Rejected: invariant A6. A class in `layout/`. Rejected: `molecule/` would then
  include a header of `layout/`, and the dependency runs the other way.

## Consequences

- A change of the rule is one class and the table of its test; the picture and the brackets follow together.
- The Ketcher canvas and an Indigo picture agree only where both follow the ticket alike. Nothing in Indigo
  checks that.
- Pictures changed: a substituent below its neighbour on a vertical bond is written `H3C` and `HO`; hydrogens
  stand above or below a symbol where they stood beside it and the other way round; they are no longer moved
  away from a close neighbour and can meet its label.
- Brackets placed by the layout changed wherever hydrogens moved, and so did the spacing of a reaction laid out
  with a font size.
- `ElementHydrogenOnLeft` stays, as the list for atoms without bonds, with the class as its only caller. The
  alignment of pseudo-atom labels keeps the renderer's former weights under a name of its own.
