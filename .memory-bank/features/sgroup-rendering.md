# Brackets and labels of S-groups in a picture

> **Read when:** a ticket is about how an S-group looks in PNG, SVG, PDF or EMF: its brackets, their hooks, its
> label or its repeat pattern.
> **Skip when:** the question is where the brackets stand in the structure: that is
> [sgroup-brackets.md](./sgroup-brackets.md).

Verified on `630c981f5`. Ticket #3976, under the polymer epic #3971.

## Problem

A picture showed the brackets and the label of repeating units, multiple groups, superatoms and generic groups
only. A copolymer, a component, a mixture or a formulation had its brackets in the file and nothing in the
picture: a block copolymer of two repeating units showed the two inner pairs with `n` and nothing around them.
A structure without bracket coordinates got a pair placed by rules of the renderer's own, not those of the
layout, and the hooks of a bracket grew with its length.

## Interface

Nothing to call: every rendering of a molecule or a reaction, in every output format, with no option.

## Expected behaviour

**When** a bracketed S-group (repeating unit, copolymer, component, mixture, formulation) has brackets, **then**
they are drawn where they are, in any direction; a zero-length bracket is not drawn.

**When** such a group has no brackets, or only zero-length ones (CDX, CDXML, CXSMILES, a structure built through
the API), **then** it is drawn with brackets placed by the rules of [sgroup-brackets.md](./sgroup-brackets.md),
at the default font and in the label mode of the rendering; the structure itself is not changed.

**When** a bracket of a bracketed group or of a multiple group is drawn, **then** its hooks are 0.15 bond lengths
long, perpendicular to the bracket and turned towards the centre of the group's atoms. The hooks of generic
groups and superatoms stay 0.15 of the bracket's length, as before.

**When** a group has a label, **then** it stands at the lower end of the closing bracket: its top edge level
with that end, its left edge 0.05 bond lengths right of the bracket, horizontal whatever the bracket's direction.
The closing bracket is the rightmost one by its middle, of brackets one above the other the lower one; the lower
end is the one lower on the page, of a horizontal bracket its left end. This holds for brackets stored in any
order, swapped in the file or turned with the molecule. The labels: a repeating unit its label from the file or
`n`; a copolymer `co`, or `alt`, `ran`, `blk` by its subtype; a component `c`, or `cN` by its order; a mixture
`mix`; a formulation `f`; a multiple group its multiplier. A superatom's label stays where it was: its bottom
edge level with the lower end, a line width right of the bracket. A generic group has no label.

**When** a repeating unit or a copolymer has a repeat pattern other than head-to-tail, **then** `hh`
(head-to-head) or `eu` (either, unknown, or not set) stands at the upper end of the closing bracket: its bottom
edge level with that end, the same offset to the right.

**When** groups are nested, **then** all of them are drawn. Labels and repeat patterns do not move to make room:
they may overlap the bracket of a parent or of a neighbouring group.

**When** a data S-group has a value, **then** it is drawn where its position says, as before. A relative value
without atoms of its own, whose parents give it no frame, is not drawn; before this change it had no position
and was not visible either.

## Guarantees

Deterministic for given coordinates; the same drawing in PNG, SVG, PDF and EMF. The forty structures of the
ticket are pinned as SVG in the integration harness, one reference for the CI platforms, since the glyphs come from
the font built into the renderer; an ARM Mac differs in the sixth decimal of the coordinates.

## Limitations

- A repeating unit of one atom without a label has brackets 0.2 bond lengths apart, so their hooks overlap.
- Nothing keeps a label clear of the bracket of a neighbouring group, or a bracket clear of a label standing
  within its margin.
- Brackets placed by the renderer for a group without them use the default font, not `render-font-size`.
- A generic or multiple group whose file has fewer than two real brackets gets the renderer's own pair, three
  line widths outside its atom labels, as before.
- The picture of an unnamed superatom no longer reserves room for its empty label, so it is a little smaller
  than before.
