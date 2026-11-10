# S-groups in the renderer

> **Read when:** you touch `core/render2d/render_sgroups.*` or the way `MoleculeRenderInternal` draws S-groups,
> or you add the drawing of an S-group type.
> **Skip when:** you only need the promised picture: that is
> [../features/sgroup-rendering.md](../features/sgroup-rendering.md).

Verified on `630c981f5` (ticket #3976).

## Responsibility

Turns the S-groups of a molecule into the items the renderer draws: brackets with hooks, labels, repeat
patterns, data values. It does not decide where brackets stand in the structure (the layout does,
[layout-sgroup-brackets.md](./layout-sgroup-brackets.md)); for a bracketed group that arrives without brackets
it runs the same layout class on the renderer's private copy of the molecule.

## Public interface

Three classes in `core/render2d/render_sgroups.h`, used by `MoleculeRenderInternal::_initSGroups` only:

- `SGroupRenderer::create(sgroup)` gives the drawer of a type, `nullptr` for a type that is not drawn (MON,
  MER, CRO, MOD, GRA, ANY). `draw(sgroup, frame, canvas)` adds the items of the group; `frameForChildren` is
  what a relative data value without atoms of its own is drawn from, passed down the tree of groups.
- `SGroupCanvas`: what a drawer may know and do. Atom positions and label boxes on the page, the
  structure-to-page transform, lengths on the page (`toDrawingLength`, `lineWidth`), and adding a bracket or a
  measured text to the group being drawn.
- `SGroupBrackets`: the rules read off brackets already on the page: the closing bracket, the lower and the
  upper end, the side of the hooks.

To draw a type differently: a subclass that overrides what differs (`_brackets`, `_hookLength`, `_labelOffset`,
`_placeLabel`, `_label`, `_repeatPattern`, `frameForChildren`, or `draw` itself as `DataSGroupRenderer` does)
and one line in `create`.

## Dependencies / Dependents

Reads `SGroup` and its subclasses; writes `MoleculeRenderData` (`sgroups`, `brackets`, `textitems`); measures
text through `RenderContext::setTextItemSize`. `MoleculeRenderInternal::_placeMissingBrackets` calls
`SGroupBracketLayout::update()` on the renderer's copy. The items are drawn by `_renderSGroups`, which the
change did not touch.

## Constraints & traps

- **Page coordinates have y down.** The ticket states its rules with y up; `SGroupBrackets` is the one place
  where "lower" means the greater y and "upper" the smaller. A label position computed anywhere else must go
  through its functions, or it lands on the wrong end of a swapped or turned bracket.
- **Two units of length.** `SGroupRenderer::LABEL_OFFSET` and `BRACKET_HOOK` are in molecule units
  (`LayoutOptions::DEFAULT_BOND_LENGTH`) and become pixels through `SGroupCanvas::toDrawingLength`; the
  renderer's own pair around atom labels and the superatom's label offset are in line widths (`lineWidth`),
  as they were before the change.
- **A text item is placed by its top left corner** (`bbp`), after `setTextItemSize` has measured it:
  `SGroupCanvas::addText` measures, the caller places. Anything anchored at the bottom needs `bbsz.y`, so it
  cannot be placed before the measurement.
- **The molecule is copied** when a bracketed group lacks brackets (`_placeMissingBrackets`), as it already is
  for contracted superatoms and multiple groups (`_prepareSGroups`); the brackets go to the copy, the caller's
  molecule is not changed. A multiple group is collapsed before drawing, so its drawer sees the remaining atoms.
- **The colour property is not a drawing.** A data S-group named by `render-atom-color-property` colours its
  atoms and is skipped, children included, before any drawer runs (`MoleculeRenderInternal::_colorsAtoms`).
- **An unnamed superatom no longer gets an empty text item.** Before the change that item widened the picture;
  references of such pictures are a little smaller now (`sa-mono` in the rendering tests).
- **A relative data value with no origin is not drawn.** Before the change it was added with a NaN position;
  now `DataSGroupRenderer::draw` returns before adding it.
