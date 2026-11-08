# Data S-group values in the layout

> **Read when:** a ticket is about where the value of a data S-group stands after `layout()` or `clean2d()`, or you
> touch `core/indigo-core/layout/sgroup_layout*`, `Transform3f::bestFit` or anything that moves atoms.
> **Skip when:** you only need the promised behaviour: that is
> [../features/sgroup-data-values.md](../features/sgroup-data-values.md).

Verified on 2026-11-08.

## Responsibility

Moves the stored position of a data S-group's value after the atoms moved, so that it keeps its place against
them. It does not place a value that has no stored position (the renderer draws an attached value next to the
first atom), and it leaves a relative value of a group without atoms (see the traps).

## Public interface

`SGroupLayout`, the class that also places brackets: one snapshot of the atoms at construction, one `update()`
after the move (`core/indigo-core/layout/sgroup_layout.h#void update()`). The table of callers is in
[layout-sgroup-brackets.md](./layout-sgroup-brackets.md). For the values they differ as follows:
`MoleculeLayout::_make` and `MoleculeCleaner2d::do_clean` change the shape of a group, and only there the fit runs on
absolute values; `ReactionLayout` and `PathwayLayout` move whole molecules, which `transformPositions` has already
done for an absolute value; the loaders move nothing.

What a stored position means lives on the group, not here: `core/indigo-core/molecule/molecule_sgroups.h#originAtom`,
which the renderer reads too (`core/render2d/src/render_internal.cpp#originAtom`),
`core/indigo-core/molecule/molecule_sgroups.h#displayPoint`,
`core/indigo-core/molecule/molecule_sgroups.h#setDisplayPoint` and
`core/indigo-core/molecule/molecule_sgroups.h#transformPosition`.

## How a value is moved

`core/indigo-core/layout/src/sgroup_layout.cpp#_moveValues`:

1. **Reference atoms**: the group's own atoms; without them, the own atoms of its parent as
   `core/indigo-core/molecule/molecule_sgroups.h#resolveParents` finds it, the one rule for S-group parents
   ([../adr/sgroup-parent-links.md](../adr/sgroup-parent-links.md)), called once per `update()`. Without either the
   value stays.
2. If no reference atom moved farther than 0.001 (`core/indigo-core/layout/src/sgroup_layout.cpp#_standApart`), the
   value stays bit for bit, so a second `layout()` changes nothing.
3. If the value itself stands apart from where it stood at construction, a whole-molecule move has carried it, and it
   stays there.
4. **The fit**: `core/indigo-core/common/math/best_fit.cpp#Transform3f::bestFit` from the old positions of the
   reference atoms to the new ones, both flattened to z = 0: the least-squares rotation, one scale and a shift. On a
   flat set a mirror comes out as a half turn about an axis in the plane, so the points stay flat. One atom or atoms at
   one point give a shift only.
5. The fit moves the point the position marks; a relative position is stored back as an offset from the new first
   atom, so a drawing moves alike whichever way its value is stored.

## Constraints & traps

- **Reference atoms on a straight line** before or after the layout (a chain drawn straight, a nitrile) fix only the
  turn of that line, and `bestFit` finds the one turn that matches the atoms along it best
  (`core/indigo-core/common/math/best_fit.cpp#Transform3f::bestFit`). Without that branch the second axis of the fit
  is zero or rounding noise: the value falls onto the line, or, with coordinates rounded to 4 decimals, jumps to the
  other side of it, and the rms reads 0. The test for a line is the sum of squares off it against an absolute bound,
  not a ratio of the two axes: a long flat zigzag has a small ratio and must not be taken for a line. A line in
  reverse order comes out as a half turn on the sheet; in 3D it is a point inversion, since `rotationVecVec` treats
  antiparallel vectors as codirectional and the scale goes negative. `indigoAlignAtoms` and the exact match with an
  rms threshold use the same fit: `api/tests/integration/tests/basic/align_atoms.py` pins the line cases, the
  mirrored long chain and a chain out of the sheet.
- **A single atom has no scale.** In `layout()` such a value keeps its distance in coordinates: with bonds longer than
  1.0 it ends farther from the atom in bond lengths, with shorter ones closer. In a reaction or a pathway step 3 keeps
  the scale of the move.
- **The fitted scale is not the change of the bond length** when the layout changes the shape of the group. On the 68
  groups taken from `layout/molecules` it was 0.36 to 1.85 of the ratio of the bond lengths, the median 1.00; on
  `data-distorted-copolymer.mol` it is 0.63 and 0.51.
- **`clean2d()` fits each pass.** `MoleculeCleaner2d::clean` runs two passes, each with its own `SGroupLayout`. On a
  strongly distorted drawing the result differs from one fit over both passes: by 0.05 and 0.07 bond lengths on
  `data-distorted-copolymer.mol`.
- **A relative position is an offset from the first atom of the group**, as the renderer resolves it. For a group
  without atoms the offset is from the right top corner of a box the renderer hands down its tree of groups
  (`core/render2d/src/render_internal.cpp#rightTop`): a repeating unit passes the box of its atoms, a component,
  mixture, formulation or copolymer passes on the box it got, starting from the whole molecule, and a data, generic,
  multiple or superatom group passes none. That box does not turn and is computed anew for every drawing, so such a
  value is left as it is.
- **No test sees the renderer take the wrong origin.** With the origin always the corner of the box, all of
  `rendering/` stays green: PNG similarity does not notice a moved value. `layout/sgroup_data.py` pins the stored
  positions only.
- **References with layout coordinates are generated on Linux** (invariant A10). On an ARM Mac the `clean2d` case of
  `api/tests/integration/tests/layout/sgroup_data.py` differs in the third decimal of the atoms.
