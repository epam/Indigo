# The side of implicit hydrogens is one rule in the core, taken from Ketcher

**Date:** 2026-11-18 · **Status:** Accepted · **Since:** Indigo 1.51

## Context

Three places decided where the hydrogens of an atom label stand. The renderer weighed four sides by the
projections of the bonds and then moved the hydrogens away from neighbouring atoms. The core's estimate of a
label box, which places the brackets of S-groups and spaces a reaction, knew left and right only. Ketcher drew
the canvas by a third rule. A picture therefore differed from the canvas, and in 18 of 82 atoms of a test set
the estimate put the hydrogens where the picture did not.

Ticket #4031 describes a common rule for both products: opposite the bond for an atom with one bond, the first
free quadrant for an atom with more. It leaves three cases to the implementation. Its hexane picture has the
hydrogens of the lower carbons below, where its requirement 2.2 gives the top. A vertical bond lies on the
boundary of its two halves, so the side would turn on the last digit of a coordinate. An atom without bonds is
neither terminal nor internal by its definitions. Ketcher's implementation, pull request epam/ketcher#12564, had
already chosen a reading of each.

## Decision

One class in the core, `ImplicitHydrogenSide`, decides the side from the directions of the atom's bonds and its
radical. The renderer and the estimate of the label box both take the side from it and keep no rule of their
own.

The rule is Ketcher's `getImplicitHydrogenSide`, with its numbers: angles rounded to 0.01°, a bond within 0.5°
of vertical counts as vertical, a quadrant includes its starting angle, the order is right, left, top, bottom,
and when top and bottom are both free the hydrogens go away from the bonds. Where Ketcher's reading differs from
the text of the ticket, Indigo follows Ketcher, because the purpose is the same picture in both.

The renderer no longer moves hydrogens away from neighbouring atoms, and it draws every radical above the
symbol, as Ketcher does.

## Alternatives considered

- Keep the renderer's weights and teach the estimate the same weights. Rejected: the picture would still differ
  from the canvas, which is what the ticket is about.
- Follow the text of the ticket where Ketcher departs from it. Rejected: each such place would be a difference
  between the picture and the canvas, decided by Indigo alone.
- Keep the move away from neighbouring atoms. Rejected: Ketcher has none, and with it the side depends on more
  than the atom's bonds, so the estimate could not share the rule without searching for neighbours too.
- Keep the hydrogens of an atom without bonds on the left for O, S and the halogens (`H2O`, `HCl`), the usual
  way of writing them. Rejected for parity: Ketcher's pull request puts them on the right.
- A method of `BaseMolecule`. Rejected: invariant A6. A class in `layout/`. Rejected: `molecule/` would then
  include a header of `layout/`, and the dependency runs the other way.

## Consequences

- A change of the rule is one class and the table of its test; the picture and the brackets follow together.
- Ketcher and Indigo stay alike only while the two implementations are changed together (invariant A19). The
  pull request was still open when this was written; a change in its review has to be repeated here, with the
  references.
- Pictures changed: an atom without bonds shows `OH2`, `ClH`, `SH2`; an atom whose two bonds point to the same
  side can have its hydrogen between them, inside a ring; hydrogens can overlap the label of a close neighbour.
- Brackets placed by the layout changed wherever hydrogens stand above or below a symbol or moved to the other
  side of it, and so did the spacing of a reaction laid out with a font size.
- `ElementHydrogenOnLeft` is gone: nothing decides a side by the element any more. The alignment of pseudo-atom
  labels kept the renderer's former weights under a name of its own.
