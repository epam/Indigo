# S-groups are drawn by one renderer class per type over a shared default

**Date:** 2026-11-10 · **Status:** Accepted · **Ticket:** #3976

## Context

`MoleculeRenderInternal::_initSGroups` was one function with a branch per S-group type. Each branch computed
brackets, hooks and the label position with its own arithmetic, the label went to the bracket written last,
and the copolymer, component, mixture and formulation types had no branch at all. The ticket adds the drawing
of those four types and changes the rules for the rest: hooks of a fixed length towards the atoms, the label
at the lower end of the closing bracket found by position, the repeat pattern at its upper end. The team lead
asked for a structure in which a parent class draws by default and a type with a custom drawing overrides it.

## Decision

A `SGroupRenderer` base class draws an S-group by the rules of the ticket. One subclass per drawn type
overrides only what differs: the label text, the repeat pattern, the source of the brackets, the hook length,
the label offset and placement, or the whole drawing for data S-groups. A factory maps the type to its class
and returns nothing for a type that is not drawn. The drawers see the molecule drawing through `SGroupCanvas`
and never through `MoleculeRenderInternal`; the rules that read a bracket on the page (closing bracket, ends,
hook side) live in `SGroupBrackets`, in page coordinates.

The renderer places no brackets of its own for bracketed groups: a group that arrives without brackets gets
them from the layout's `SGroupLayout`, run on the renderer's copy of the molecule.

## Alternatives considered

- Keep the single function and add four branches. Rejected: the label and hook arithmetic would exist in nine
  copies, and the lead asked for the class structure.
- One class with a label table for the types that differ only by their label text (component, mixture,
  formulation). Rejected for uniformity: a subclass per type is a few lines, and a custom drawing for any type
  stays a one-step change.
- Brackets for groups without them by the renderer's former rule, a pair three line widths outside the atom
  labels. Rejected: the ticket requires the layout's rules, so that a structure looks the same whether its
  brackets were placed on load or at drawing time.

## Consequences

- A new drawn type is a subclass and a line in the factory; a change to the common rules is one place.
- The order and direction of brackets in a file no longer matter to the picture; invariant A12 is now a
  writing convention.
- The former drawing of generic groups and superatoms is preserved through overrides, and a change to the base
  must keep them as they are.
- Rendering a molecule with a bracketed group without brackets copies the molecule, as contracted superatoms
  already did.
