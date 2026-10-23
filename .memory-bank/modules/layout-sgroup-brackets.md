# S-group brackets in the layout

> **Read when:** a ticket is about where the brackets of a polymer S-group stand, or you touch
> `core/indigo-core/layout/sgroup_*`, `bracket_spacing.*`, `atom_boxes.*`, or anything that moves atoms.
> **Skip when:** you only need the promised behaviour: that is
> [../features/sgroup-brackets.md](../features/sgroup-brackets.md).

Verified on `14018d3d3` (ticket #3974).

## Responsibility

Computes the two brackets of every **bracketed** S-group, SRU, COP, COM, MIX or FOR
(`core/indigo-core/layout/src/sgroup_nesting.cpp#isBracketed`), from the coordinates of the atoms, and
decides when a group gets new brackets. It does not draw them (render2d), does not place the brackets of a
multiple group (`core/indigo-core/layout/src/molecule_layout.cpp#_placeSGroupBracketsHorizontal`) or of a
superatom (the renderer's own rules), and knows nothing about render options: labels are measured at the
renderer's default font, 0.4 of a bond length, in the terminal-hetero label mode.

## Public interface

One class, two calls. `SGroupBracketLayout layout(molecule)` remembers where the atoms stand;
`layout.update()` places brackets for the groups that have none (`SGroup::hasBrackets()` is false) and for
those whose own atoms moved since construction
(`core/indigo-core/layout/sgroup_bracket_layout.h#void update()`). Callers:

| Caller | Constructed | `update()` called |
| --- | --- | --- |
| `MoleculeLayout::_make`: `layout()`, reaction and R-group layout | before the layout graph runs | after the atoms and the multiple-group brackets are written back |
| `MoleculeLayout::updateSGroups`: the KET loader | at once | at once: every group there has placeholders |
| `MoleculeCleaner2d::do_clean`: `clean2d()`, both passes | after the trivial-structure returns | after the coordinates are written back |
| `MolfileLoader::_postLoad` | at once | at once: only groups without `SDI` / `BRKXYZ` change |

Moves that keep the drawing rigid do not re-place: `Metalayout::adjustMol`, `BaseMolecule::scale` and
`BaseMolecule::offsetCoordinates` carry every group's brackets along through
`core/indigo-core/molecule/molecule_sgroups.h#transformBrackets` (invariant A13).

## How a group is placed

Groups are taken deepest first (`SGroupNesting::bracketedDeepestFirst`), so a parent sees the brackets of its
children. For one group:

1. **Bounding box**: the union of its atoms' boxes (`AtomBoxes`: the centre of an atom drawn without a label,
   the label box otherwise, the hydrogens above or below where the renderer puts them,
   `core/indigo-core/molecule/base_molecule.h#multiline_labels`) and of the boxes spanned by the children's
   brackets.
2. **Each side** stands `BracketSpacing::STEP` (0.3 bond lengths) outside the bounding box, unless the nearest
   atom outside the group on that side is too close. Then the brackets that stand between the group and that
   atom (`SGroupNesting::groupsBetween`, innermost first) share the gap: one reduced step for every interval,
   not below `MIN_STEP`; next to a lone atom such as the `*` at a chain end the inner brackets stay before it
   and the outer ones pass it; between two groups the step is `min(STEP, gap / n)`
   (`core/indigo-core/layout/bracket_spacing.h#distances`). Above and below, the group is alone in its gap.
3. The sides keep `TEXT_CLEARANCE` from the children's brackets and texts (the label at the lower right of the
   closing bracket, the repeat pattern at the upper right, `core/indigo-core/layout/sgroup_texts.h#box`), move
   past any lone atom their line would run through, and the brackets are lengthened to one bond length when
   shorter.

The algorithm transcribes the reference planner of the ticket formula by formula. Its mock-ups are the
fixtures
`api/tests/integration/tests/layout/molecules/sgroups/f01-sru-straight.mol` to `f14-three-on-one-bond.mol`,
pinned as loaded and after `layout()` by `api/tests/integration/tests/layout/sgroup_brackets.py`.

## Nesting

`SGroupNesting` takes the parent from the file when it is a bracketed group containing the child's atoms,
otherwise the smallest bracketed group strictly containing them
(`core/indigo-core/layout/src/sgroup_nesting.cpp#_findParent`). Two groups with the same atoms and no parent
link are **not** nested, and their brackets coincide. A parent loop from the file is cut. A multiple group can
be a child, never a parent.

## Constraints & traps

- **The unit is the mean bond length**, zero-length bonds (the unexpanded copies of a multiple group) excluded
  (`core/indigo-core/layout/src/sgroup_bracket_layout.cpp#_averageBondLength`). One or two very long bonds,
  a `*` drawn far away, inflate every distance. The median was considered and rejected: the reference planner
  and the rest of the library use the mean.
- **Loaded brackets are kept while the group's atoms stay** (requirement 10 of the ticket), even when a child
  gets new brackets underneath them. A layout of a selection therefore re-places only the groups it moved,
  including the unselected fragments the layout packs anew.
- **Label widths are estimates**: 0.7 of the font size per character, the count of the hydrogens to the right
  of an `H` centred above the atom. A bracket stands 0.3 from that box, which a drawing shows as more than 0.3
  from a narrow glyph such as `H3C`.
- **References with layout coordinates are generated on Linux** (invariant A10). On an ARM Mac two `clean2d`
  references of `sgroup_brackets.py` differ in the fourth decimal.
- The order of the two brackets is a contract with the renderer (A12); zero-length brackets are placeholders
  (A14).
