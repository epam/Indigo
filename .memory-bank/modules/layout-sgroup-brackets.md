# S-group brackets in the layout

> **Read when:** a ticket is about where the brackets of a polymer S-group stand, or you touch
> `core/indigo-core/layout/sgroup_*` or anything that moves atoms.
> **Skip when:** you only need the promised behaviour: that is
> [../features/sgroup-brackets.md](../features/sgroup-brackets.md).

Verified on `1e15006f7` (ticket #3974).

## Responsibility

Computes the two brackets of every **bracketed** S-group, SRU, COP, COM, MIX or FOR
(`core/indigo-core/layout/src/sgroup_nesting.cpp#isBracketed`), from the coordinates of the atoms, and
decides when a group gets new brackets. It does not draw them (render2d), does not place the brackets of a
multiple group (`core/indigo-core/layout/src/molecule_layout.cpp#_placeSGroupBracketsHorizontal`) or of a
superatom (the renderer's own rules), and looks at nothing outside the group: not at neighbouring atoms, not
at the labels of nested groups.

## Public interface

One class, two calls. `SGroupBracketLayout layout(molecule, font_size, label_mode)` remembers where the atoms
stand; `layout.update()` places brackets for the groups that have none (`SGroup::hasBrackets()` is false) and
for those whose own atoms moved since construction
(`core/indigo-core/layout/sgroup_bracket_layout.h#void update()`). The font size is in the coordinates of
the molecule and defaults to the renderer's font at the default bond length
(`core/indigo-core/layout/sgroup_bracket_layout.h#DEFAULT_FONT_SIZE`), the label mode to terminal-hetero.
Callers:

| Caller | Constructed | `update()` called | Font |
| --- | --- | --- | --- |
| `MoleculeLayout::_make`: `layout()`, reaction, pathway and R-group layout | before the layout graph runs | after the atoms and the multiple-group brackets are written back | `render-font-size`, `render-label-mode` of the session through `core/indigo-core/layout/molecule_layout.h#setLabelOptions`, called by `api/c/indigo/src/indigo_layout.cpp#setLabelOptions`, `ReactionLayout` and `PathwayLayout` |
| `MoleculeLayout::updateSGroups`: the KET loader | at once | at once: every group there has placeholders | default |
| `MoleculeCleaner2d::do_clean`: `clean2d()`, both passes | after the trivial-structure returns | after the coordinates are written back | default |
| `MolfileLoader::_postLoad` | at once | at once: only groups without `SDI` / `BRKXYZ` change | default |
| `ReactionLayout::make`, `PathwayLayout::make`: the metalayout's move and scale of whole molecules | before `Metalayout::process()` / `buildLayoutTree()` | after `process()` / `applyLayout()` | the same options |

`Metalayout::adjustMol`, `BaseMolecule::scale` and `BaseMolecule::offsetCoordinates` carry every group's real
brackets along through `core/indigo-core/molecule/molecule_sgroups.h#transformBrackets` (invariant A13); that is
what the brackets of the other S-group types get, and what the reaction and pathway layouts then override for
the bracketed types, because a scale changes the margins.

## How a group is placed

Groups are taken deepest first (`SGroupNesting::bracketedDeepestFirst`), so a parent sees the brackets of its
children. For one group (`core/indigo-core/layout/src/sgroup_bracket_layout.cpp#_bracketsBox`):

1. **Atoms box**: the union of its atoms' boxes, each the centre of an atom drawn without a label and the box of
   its label otherwise, as `core/indigo-core/molecule/src/base_molecule_properties.cpp#getAtomBoundingBox`
   estimates it (`core/indigo-core/layout/src/sgroup_bracket_layout.cpp#_atomsBox`).
2. **Margin and space**: the atoms box grows by the margin, 0.1 bond lengths to the left and the right and 0.3
   up and down; the box spanned by each child's brackets grows by the space, 0.2 on every side. The brackets
   stand on the left and the right side of the union of these boxes, so whichever reaches further wins
   (`core/indigo-core/layout/sgroup_bracket_layout.h#HORIZONTAL_MARGIN`). A group without children gets
   exactly the margin; a parent whose children cover its atoms gets exactly the space past their brackets.

That is the whole rule. The ticket's two pictures are the fixtures
`api/tests/integration/tests/layout/molecules/sgroups/ticket-single-sru.mol` and
`api/tests/integration/tests/layout/molecules/sgroups/ticket-nested-copolymer.mol`, pinned as loaded and after `layout()` by
`api/tests/integration/tests/layout/sgroup_brackets.py`, next to the fourteen earlier mock-ups.

## Nesting

`SGroupNesting` takes the parent from the file when it is a bracketed group containing the child's atoms,
otherwise the smallest bracketed group strictly containing them
(`core/indigo-core/layout/src/sgroup_nesting.cpp#_findParent`). Two groups with the same atoms and no parent
link are **not** nested, and their brackets coincide. A parent loop from the file is cut. A multiple group can
be a child, never a parent.

## Constraints & traps

- **The unit is `LayoutOptions::DEFAULT_BOND_LENGTH`**, not the bond length of the drawing: the ticket asks for
  the existing constant. A structure drawn at another scale gets margins of 0.1 and 0.3 absolute, out of
  proportion with its bonds.
- **Nothing outside the group is looked at.** A bracket runs through a neighbouring atom, the `*` at a chain
  end or a label when it stands within the margin; the texts of a child (`n`, the repeat pattern, #3976) are not
  part of the child's bracket box, so a parent bracket 0.2 away can touch them. There is no least length: a
  one-atom group without a label gets brackets 0.6 long.
- **The font comes from the layout options only in `layout()`.** The option is a size relative to the bond
  length (`core/indigo-core/layout/metalayout.h#getFontSizeInAngstroms`); 40 pt at the default 100 px bond is
  the default 0.4. Loaders and `clean2d()` have no options and measure at the default, so the brackets a KET
  structure gets on load can differ from those after `layout()` with a font option.
- **Label widths are estimates**: 0.7 of the font size per character, hydrogens to the left or the right only,
  as the renderer's own estimate does. The font type is not consulted.
- **Loaded brackets are kept while the group's atoms stay** (requirement of the ticket), even when a child gets
  new brackets underneath them. A layout of a selection therefore re-places only the groups it moved, including
  the unselected fragments the layout packs anew.
- **References with layout coordinates are generated on Linux** (invariant A10). On an ARM Mac two `clean2d`
  references of `sgroup_brackets.py` differ in the fourth decimal.
- The order of the two brackets is a contract with the renderer (A12); zero-length brackets are placeholders
  (A14).
