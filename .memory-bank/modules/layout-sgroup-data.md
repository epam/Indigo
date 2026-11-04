# Data S-group values in the layout

> **Read when:** a ticket is about where the value of a data S-group stands after `layout()` or `clean2d()`, or you
> touch `core/indigo-core/layout/sgroup_data_layout*` or anything that moves atoms.
> **Skip when:** you only need the promised behaviour: that is
> [../features/sgroup-data-values.md](../features/sgroup-data-values.md).

Verified on `da78213ac` (ticket #3975).

## Responsibility

Moves the stored position of a data S-group's value after the atoms moved, so that it keeps its place against
them. It does not place a value that has no stored position (the renderer draws an attached value next to the
first atom), it leaves a relative value of a group without atoms (see the traps), and it never scales or mirrors:
the ticket asks for a turn and a shift only.

## Public interface

One class, two calls, the lifecycle of `SGroupBracketLayout`. `SGroupDataLayout layout(molecule)` remembers the
atoms and every stored value; `layout.update()` moves the values whose reference atoms moved since construction
(`core/indigo-core/layout/sgroup_data_layout.h#void update()`). Callers:

| Caller | Constructed | `update()` called |
| --- | --- | --- |
| `MoleculeLayout::_make`: `layout()`, and every molecule laid out inside a reaction, a pathway or an R-group layout | before the layout graph runs, next to `SGroupBracketLayout` | after the atoms are written back |
| `MoleculeCleaner2d::do_clean`: each of the two passes of `clean2d()`, and the single pass of a selection | after the trivial-structure returns | after the coordinates are written back |

Whole-molecule moves (`Metalayout::adjustMol`, `BaseMolecule::scale`, `BaseMolecule::offsetCoordinates`) carry the
values through `core/indigo-core/molecule/molecule_sgroups.h#transformPositions` instead (invariant A13).

## How a value is moved

1. **Reference atoms**: the group's own atoms; without them, the own atoms of its parent as
   `core/indigo-core/molecule/molecule_sgroups.h#resolveParents` finds it, the one rule for S-group parents
   ([../adr/sgroup-parent-links.md](../adr/sgroup-parent-links.md)), called once per `update()`. Without either the
   value stays.
2. If no reference atom moved by more than `EPSILON`, the value stays bit for bit, so a second `layout()` changes
   nothing.
3. **The fit**: the rotation and the shift that bring the old positions of the reference atoms closest to the new
   ones by least squares, from the sums of dot and cross products around the two centres. One atom, atoms at one
   point, or a fit weaker than a thousandth of a perfect one give a shift only
   (`core/indigo-core/layout/sgroup_data_layout.h#WEAKEST_FIT`). The only elementary function is `HYPOT`
   (invariant A10).
4. The position turns about the old centre and moves to the new one. A relative one is first made the point it marks
   (the old first atom plus the offset) and is stored back as an offset from the new first atom, so a drawing moves
   alike whichever way its value is stored. The position remembered at construction is the one moved, so a
   whole-molecule move in between is not applied twice.

## Constraints & traps

- **Not in `ReactionLayout` or `PathwayLayout`.** They move and scale whole molecules, and `transformPositions`
  already carries the values with the scale. A fit there, having no scale, would put the value of a scaled molecule
  back at its old distance.
- **`clean2d()` fits each pass.** `MoleculeCleaner2d::clean` runs two passes, each with its own `SGroupDataLayout`.
  On a strongly distorted drawing the result differs from one fit over both passes: by 0.05 and 0.14 bond lengths
  on the ticket's file.
- **`MoleculeSGroups::getParentAtoms` is not the parent's atoms.** It concatenates the atoms of every ancestor, so a
  value on a component would follow the whole formulation.
- **A relative position is an offset from the first atom of the group**, as the renderer resolves it. Turning only
  the offset would leave the value by the first atom when the layout changes the shape of the group, and a value
  stored as absolute would go elsewhere. For a group without atoms the offset is from the right top corner of a box the renderer takes from the groups above it: the atoms of the nearest repeating
  unit, or the whole molecule (`core/render2d/src/render_internal.cpp#rightTop`). That box does not turn and is
  computed anew for every drawing, so such a value is left as it is.
- **A layout that mirrors a structure** turns the value by the closest rotation instead of reflecting it. For a
  mirrored symmetric group, such as a regular ring, every turn fits equally: the sums are zero, and in float their
  rounding would pick a random turn (about one case in six for a hexagon, by a float32 simulation), so the value is
  only shifted.
- **No scale in `layout()`.** A structure drawn at another bond length is brought to 1.0, but a value keeps its
  distance from its atoms in coordinates: in `layout/ref/test_sgroup.rxn` a value 1.76 bond lengths away stands 1.46
  away after the layout.
- **References with layout coordinates are generated on Linux** (invariant A10). On an ARM Mac the `clean2d` case of
  `api/tests/integration/tests/layout/sgroup_data.py` differs in the third decimal of the atoms.
