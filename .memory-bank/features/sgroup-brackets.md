# Brackets of polymer S-groups

> **Read when:** a ticket is about where the brackets of a repeating unit, copolymer, component, mixture or
> formulation stand, or when they appear or move.
> **Skip when:** the question is about superatoms, multiple groups or data S-groups.

Verified on `1e15006f7`. Ticket #3974, under the polymer epic #3971.

## Problem

A polymer structure from KET, or a MOL file written without bracket coordinates, has S-groups with no brackets
or with zero-length placeholders. Before this change `layout()` placed brackets for repeating units only and
left those of copolymers, components, mixtures and formulations where the atoms had been, and `clean2d()`
moved no brackets at all. Brackets must show which atoms belong to a group, in the same place whichever way
the structure arrived.

## Interface

Nothing to call. Brackets are computed when a structure is loaded from MOL or KET, after `layout()` and after
`clean2d()` of a molecule, a selection, a reaction or a pathway. They are saved to MOL V2000 (`M  SDI`) and
V3000 (`BRKXYZ`). `layout()` measures atom labels at the session's `render-font-size` (with
`render-font-size-unit`) and `render-label-mode`.

## Expected behaviour

**When** a bracketed S-group (SRU, COP, COM, MIX, FOR) has no brackets or zero-length ones, **then** it gets
two vertical brackets, 0.1 bond lengths left and right of the box of its atom labels, running from 0.3 below
that box to 0.3 above it, the opening bracket on the left.

**When** a group is nested in another, **then** the parent's brackets stand 0.3 outside the child's brackets
on every side, or 0.1 and 0.3 outside the parent's own atoms where those reach further.

**When** `layout()` runs with `render-font-size` or `render-label-mode` set, **then** the labels are measured
at that font and in that mode, and the brackets move out with wider labels. Without the options, and always
on load and in `clean2d()`, labels are measured at the renderer's default font, 0.4 of a bond length, in the
terminal-hetero mode.

**When** a structure is loaded with brackets, **then** they are kept as they are while the atoms of their
group do not move, even when they do not follow these rules.

**When** `layout()` or `clean2d()` moves the atoms of a group, **then** the group gets new brackets; a second
`layout()` of the same structure moves them no more.

**When** a reaction or a pathway is laid out, **then** every bracketed group of a molecule the layout moved or
scaled gets brackets anew by these rules, also with `layout-preserve-existing`; the brackets of other S-group
types move and scale with their molecules.

**When** a structure is saved to MOL, **then** zero-length brackets are not written, for any S-group type; a real
bracket next to one is written, and a group left with fewer than two real brackets is drawn with the renderer's
own pair.

## Guarantees

Deterministic for given coordinates. The two structures pictured in the ticket and the fourteen earlier
mock-ups are pinned to four decimals.

## Limitations

- The renderer draws brackets only for repeating units, multiple groups, superatoms and generic groups; those
  of copolymers, components, mixtures and formulations are in the file but not in the picture (#3976).
- Distances are in the default bond length, 1.0, whatever the bond length of the drawing: a structure drawn
  at another scale gets margins out of proportion.
- Nothing outside the group is considered: a bracket can run through a neighbouring atom, the `*` at a chain
  end or a label standing within 0.1 of the group; the labels and repeat patterns of nested groups are not
  kept clear, and a one-atom group without a label gets brackets 0.6 long.
- Labels are measured with an estimated glyph width, 0.7 of the font size per character; the font type is
  not consulted.
- Two groups with the same atoms and no parent link in the file get coinciding brackets.
- A parent-link loop in a MOL file loads, but saving and cloning the structure still crash (pre-existing).
- Rigid moves outside the layout (aligning atoms, the standardizer's centering, unfolding hydrogens) leave
  brackets behind. CDX, CDXML and CXSMILES load their groups without brackets, so a MOL converted from them has no
  `M  SDI` at all.
- The layout of a reaction can put the components of a mixture so close that their brackets overlap (#3981).
- A multiple group inside a bracketed group is not re-placed by `clean2d()`, so its parent may stretch to its
  old brackets.
