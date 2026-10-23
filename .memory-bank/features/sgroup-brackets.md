# Brackets of polymer S-groups

> **Read when:** a ticket is about where the brackets of a repeating unit, copolymer, component, mixture or
> formulation stand, or when they appear or move.
> **Skip when:** the question is about superatoms, multiple groups or data S-groups.

Verified on `14018d3d3`. Ticket #3974, under the polymer epic #3971.

## Problem

A polymer structure from KET, or a MOL file written without bracket coordinates, has S-groups with no brackets
or with zero-length placeholders. Before this change `layout()` placed brackets for repeating units only and
left those of copolymers, components, mixtures and formulations where the atoms had been, and `clean2d()`
moved no brackets at all. Brackets must show which atoms belong to a group, in the same place whichever way
the structure arrived.

## Interface

Nothing to call. Brackets are computed when a structure is loaded from MOL or KET, after `layout()` and after
`clean2d()` of a molecule, a selection, a reaction or a pathway. They are saved to MOL V2000 (`M  SDI`) and
V3000 (`BRKXYZ`).

## Expected behaviour

**When** a bracketed S-group (SRU, COP, COM, MIX, FOR) has no brackets or zero-length ones, **then** it gets
two vertical brackets, left and right of its atoms, 0.3 bond lengths outside the box of its atom labels and
of the brackets of the groups nested in it, at least one bond length long, the opening bracket on the left.

**When** a group is nested in another, **then** the parent's brackets stand outside the child's brackets, its
label and its repeat pattern.

**When** the brackets between a group and the nearest atom outside it do not fit at 0.3, **then** they share
the gap with an equal step, never below 0.15. Next to a lone atom such as the `*` at a chain end the inner
brackets stay before it and the outer ones pass it, so that the atom is inside them; if not even one fits, it
stands in the middle of the gap.

**When** a crossing bond leaves the group, **then** it crosses the bracket wherever they meet; a bond leaving
upwards or downwards between the brackets crosses none.

**When** a structure is loaded with brackets, **then** they are kept as they are while the atoms of their
group do not move, even when they do not follow these rules.

**When** `layout()` or `clean2d()` moves the atoms of a group, **then** the group gets new brackets; a second
`layout()` of the same structure moves them no more.

**When** a reaction or a pathway is laid out, **then** the brackets move and scale with their molecules.

**When** a structure is saved to MOL, **then** zero-length brackets are not written, for any S-group type.

## Guarantees

Deterministic for given coordinates. The fourteen mock-up structures of the ticket are reproduced to four
decimals.

## Limitations

- The renderer draws brackets only for repeating units, multiple groups, superatoms and generic groups; those
  of copolymers, components, mixtures and formulations are in the file but not in the picture (#3976).
- Render options (font size, label mode, implicit hydrogens) are not consulted. Labels are measured at the
  default font with an estimated glyph width, so a bracket stands further from a wide label than the drawing
  suggests.
- Distances are in the mean bond length of the structure; a drawing with a few very long bonds gets
  proportionally larger gaps.
- Two groups with the same atoms and no parent link in the file get coinciding brackets.
- A group loaded with a single bracket keeps it, and the renderer fails on it (pre-existing).
- A parent-link loop in a MOL file loads, but saving and cloning the structure still crash (pre-existing).
- Rigid moves outside the layout (aligning atoms, the standardizer's centering, unfolding hydrogens) leave
  brackets behind. CDX, CDXML and CML load their groups without brackets.
- The layout of a reaction can put the components of a mixture so close that no bracket fits between them
  (#3981); `clean2d()` can fold a short chain so that the hooks of its brackets cross the bonds to its `*`
  ends.
- A multiple group inside a bracketed group is not re-placed by `clean2d()`, so its parent may stretch to its
  old brackets.
