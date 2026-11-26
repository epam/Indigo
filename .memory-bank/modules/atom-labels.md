# Atom labels: the side of the hydrogens, the drawn label, the estimate of its box

<!-- STUB: covers the implicit hydrogens of a label and what depends on their side. The rest of label drawing
(pseudo-atoms, query atoms, isotopes, stereo and CIP labels, attachment points) is not described. -->

> **Read when:** you touch where `H` is drawn at an atom, the size of an atom label in
> `core/render2d/src/render_internal_render.cpp#_prepareLabelText` or in
> `core/indigo-core/molecule/src/base_molecule_properties.cpp#getAtomBoundingBox`, or where a radical dot or a
> charge stands next to the hydrogens.
> **Skip when:** you only need the promised behaviour: that is
> [../features/implicit-hydrogen-position.md](../features/implicit-hydrogen-position.md).

Verified on 2026-11-24.

## Responsibility

One class decides on which side of an atom symbol the implicit hydrogens stand:
`core/indigo-core/molecule/implicit_hydrogen_side.h#ImplicitHydrogenSide`. It answers from the directions of the
atom's bonds, from whether the atom carries a radical and, for an atom without bonds, from its element. It does
not decide whether the label or its hydrogens are shown, and it does not lay the label out.

## Public interface

`ImplicitHydrogenSide(molecule, atom).side()`, and nothing else. The rule is written out in the comment of the
class.

## Dependents

- The drawing: `core/render2d/src/render_internal_render.cpp#ImplicitHydrogenSide(bm, aid)`, asked once for an
  atom whose hydrogens are shown.
- The estimate of a label box: `BaseMolecule::getAtomBoundingBox`, and through it the brackets of S-groups
  ([layout-sgroup-brackets.md](./layout-sgroup-brackets.md)) and
  `core/indigo-core/layout/src/reaction_layout.cpp#processSideBoxes`.

## Constraints & traps

- **The rule is the ticket's, with four stated departures** (invariant A19,
  [../adr/implicit-hydrogen-side-one-rule-in-the-core.md](../adr/implicit-hydrogen-side-one-rule-in-the-core.md)).
  The tables of `core/indigo-core/tests/tests/implicit_hydrogen_side.cpp#IndigoCoreImplicitHydrogenSideTest`
  pin every boundary: straight up and straight down, the four diagonals, the balance of the bonds.
- **Angles are whole hundredths of a degree** (`_bondCentidegrees`), so that the boundaries (90°, 270°, the
  diagonals) compare exactly and the noise of a float does not move a bond across one. The angle goes through
  `ATAN2` and the balance of the bonds through `COS` (invariant A10): the side moves brackets, and brackets are
  in references shared by Windows and Linux.
- **There is no tolerance around a boundary.** A bond at 90.01° is in the left half and at 89.99° in the right
  one. A tolerance was tried for the vertical and removed: whichever way it leans, it puts some bond on the side
  the rule does not give.
- **The element decides only without bonds**: `core/indigo-core/molecule/elements.h#ElementHydrogenOnLeft` is
  the list, and the class is its only caller. Asking it for an atom with bonds brings back the two rules this
  class replaced.
- **The coordinates of the molecule are the drawing.** The renderer only shifts, mirrors y and scales
  (`core/render2d/src/render_internal.cpp#_objCoordTransform`), so the drawing and the estimate ask with the
  same molecule and get the same side. Computing the side from page coordinates would need the y flip in a
  second place.
- **The side of the hydrogens is not the alignment of a label.** A pseudo-atom, a template atom or an atom list
  is turned and written leftwards by `core/render2d/src/render_internal.cpp#_labelExtendsLeft`, which keeps the
  preferences the renderer used for hydrogens before Indigo 1.51
  (`core/render2d/render_internal.h#LABEL_RIGHT_PREFERENCE` and the three next to it, less the largest
  projection of a bond above `minSin`). Feeding it the new side moves every such label.
- **A bond is trimmed against each text item of a label, not against the label's box**
  (`core/render2d/src/render_internal_render.cpp#_getBondOffset`), and hydrogens above or below the symbol are
  items like the others. A bond on the side of their count is cut short from 10° upwards, also where it passes
  clear of the count; exempting them with `noBondOffset` draws bonds and the line of an attachment point over
  the count instead. The line of an attachment point starts no further than 0.4 bond lengths from the atom, so
  with `H` above it starts inside the box of `H`, between its stems.
- **Charge and valence follow `rightMargin`.** For hydrogens above the symbol `rightMargin` moves out to the
  right edge of their count, or the sign is printed over the count. For hydrogens below it stays at the symbol:
  the count cannot meet a sign at the upper right. The estimate does the same: a count above comes before
  valence and charge, a count below only widens the box.
- **The estimate counts in characters of 0.7 of the font size**: a character is that wide, the symbol's box
  reaches that far up and down from the centre, and hydrogens above or below add one more. Measured against the
  drawing at the default font of 0.4 bond lengths: `H` above reaches 0.47 to 0.49 from the centre, the estimate
  0.56; a count with a charge reaches 0.45 to the right, the estimate 0.70.
- **A doublet stands above the symbol and moves to its upper right when a bond points up**
  (`core/render2d/src/render_internal_render.cpp#_hasBondWithin`): a bond within
  `core/render2d/render_internal.h#RADICAL_ABOVE_HALF_SECTOR` of the vertical sends it there, where it stood
  before Indigo 1.51, and hydrogens, a charge or a valence at the right keep it above. The rule looks at bond
  directions only, and the sector ends between the usual bond angles. Two richer rules were tried and dropped.
  Moving the dot whenever a bond lies in the top quadrant of the hydrogens: the place at the right is at about
  43° from the atom, so a bond at 50° runs straight into it. Measuring the dot against the lines of every bond:
  the lines of a double bond start past the label and to one side of the axis, the choice then turns on the
  width of the symbol and on rounding, and each fix of one case moved another. A singlet and a triplet stand
  above the symbol. The class counts the top as taken by any radical; when the dot moves to the right, a bond
  holds the top anyway.
- **A reaction layout measures labels only with a font size**: `_font_size < EPSILON` takes a fixed margin
  around the atom centres instead. A test of the reaction layout that does not set `render-font-size` does not
  see the hydrogens at all.
- **The SVG references of `rendering/` are generated on Linux and hold on Windows.** On an ARM Mac the outline
  of a glyph differs in its last digits (by 0.004 in the file that was compared), and every SVG comparison is
  red there.
