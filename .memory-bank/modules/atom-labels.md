# Atom labels: the side of the hydrogens, the drawn label, the estimate of its box

<!-- STUB: covers the implicit hydrogens of a label and what depends on their side. The rest of label drawing
(pseudo-atoms, query atoms, isotopes, stereo and CIP labels, attachment points) is not described. -->

> **Read when:** you touch where `H` is drawn at an atom, the size of an atom label in
> `core/render2d/src/render_internal_render.cpp#_prepareLabelText` or in
> `core/indigo-core/molecule/src/base_molecule_properties.cpp#getAtomBoundingBox`, or a picture differs from
> Ketcher at a label.
> **Skip when:** you only need the promised behaviour: that is
> [../features/implicit-hydrogen-position.md](../features/implicit-hydrogen-position.md).

Verified on 2026-11-19.

## Responsibility

One class decides on which side of an atom symbol the implicit hydrogens stand:
`core/indigo-core/molecule/implicit_hydrogen_side.h#ImplicitHydrogenSide`. It answers from the directions of the
atom's bonds and from whether the atom carries a radical, nothing else. It does not decide whether the label or
its hydrogens are shown, and it does not lay the label out.

## Public interface

`ImplicitHydrogenSide(molecule, atom).side()` for an atom of a molecule; `ImplicitHydrogenSide(bond_directions,
has_radical).side()` for bare directions, x to the right and y up; `ImplicitHydrogenSide::isStacked(side)` for
`TOP` and `BOTTOM`, where `H` stands on a line of its own.

## Dependents

- The drawing: `core/render2d/src/render_internal.cpp#ad.hydrogenSide` is set for every atom and read in
  `_prepareLabelText`.
- The estimate of a label box: `BaseMolecule::getAtomBoundingBox`, and through it the brackets of S-groups
  ([layout-sgroup-brackets.md](./layout-sgroup-brackets.md)) and
  `core/indigo-core/layout/src/reaction_layout.cpp#processSideBoxes`.

## Constraints & traps

- **The rule is Ketcher's, number for number** (invariant A19): `getImplicitHydrogenSide` in
  `packages/ketcher-core/src/domain/helpers/implicitHydrogenSide.ts`. The cases of its test are the tables of
  `core/indigo-core/tests/tests/implicit_hydrogen_side.cpp#IndigoCoreImplicitHydrogenSideTest`.
- **Angles are whole hundredths of a degree** (`_bondCentidegrees`), because Ketcher rounds to 0.01° and the
  boundaries (45°, 135°, 0.5° from vertical) must then compare exactly. The angle goes through `ATAN2` and the
  balance of the bonds through `SIN` (invariant A10): the side moves brackets, and brackets are in references
  shared by Windows and Linux.
- **The coordinates of the molecule are the drawing.** The renderer only shifts, mirrors y and scales
  (`core/render2d/src/render_internal.cpp#_objCoordTransform`), so the drawing and the estimate ask with the
  same molecule and get the same side. Computing the side from page coordinates would need the y flip in a
  second place.
- **The side of the hydrogens is not the alignment of a label.** A pseudo-atom, a template atom or an atom list
  is turned and written leftwards by `core/render2d/src/render_internal.cpp#_labelExtendsLeft`, which keeps the
  weights the renderer used for hydrogens before Indigo 1.51 (0.3 right, 0.2 left, 0.1 up, 0 down, less the
  largest projection of a bond above `minSin`). Feeding it the new side moves every such label.
- **A bond is trimmed against each text item of a label, not against the label's box**
  (`core/render2d/src/render_internal_render.cpp#_getBondOffset`). Hydrogens above or below the symbol and their
  count carry `noBondOffset`: a bond at 44° passes the corner of an `H` above the symbol and would be cut short.
  Hydrogens beside the symbol do trim the bond that runs into them.
- **Charge and valence follow `rightMargin`.** For hydrogens above or below, `rightMargin` moves out to the right
  edge of the count, above and below alike, or the sign is printed over the count. The estimate does the same:
  its hydrogen block stands before valence and charge, and with a count the right edge is at least one and a
  half characters from the centre.
- **The estimate counts in characters of 0.7 of the font size**: a character is that wide, the symbol's box
  reaches that far up and down from the centre, and hydrogens above or below add one more. Measured against the
  drawing at the default font of 0.4 bond lengths: `H` above reaches 0.47 to 0.49 from the centre, the estimate
  0.56; a count with a charge reaches 0.45 to the right, the estimate 0.70.
- **A radical is drawn above the symbol for all three kinds.** The rule treats the top quadrant as taken, so a
  radical drawn anywhere else leaves the hydrogens below an empty top.
- **A reaction layout measures labels only with a font size**: `_font_size < EPSILON` takes a fixed margin
  around the atom centres instead. A test of the reaction layout that does not set `render-font-size` does not
  see the hydrogens at all.
- **The SVG references of `rendering/` are generated on Linux and hold on Windows and x86 macOS.** On an ARM Mac
  the outline of a glyph differs in its last digits (by 0.004 in the file that was compared), and every SVG
  comparison is red there.
