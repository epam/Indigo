# Public components follow haptic bonds; graph algorithms keep the graph's components

**Date:** 2026-10-07 · **Status:** Accepted · **Ticket:** #3927

## Context

A haptic bond is not an edge ([haptic-bond-endpoints.md](./haptic-bond-endpoints.md)), so the graph
splits a ferrocene into two rings and an iron. The public component API reported that split, while
KET — through `collectExternalNeighbors` — already wrote the complex as one molecule node; the
fragment options of `standardize` and the wrappers' salt helpers removed the metal of a complex.

The graph's component cache serves its external-neighbour mode and its plain readers from one set of
arrays: after `countComponents(external_neighbors)`, `getDecomposition()` and `vertexComponent()`
return the merged decomposition until the next edge change. Its four users — the KET saver, the
reaction KET loader, the multistep detector and the reaction SMILES loader — want exactly that, and
each works on a molecule of its own. Layout (`MoleculeLayout::_atomToAtomHapticRings`), InChI and the
automapper read the plain readers on the caller's molecule.

## Decision

`BaseMolecule::moleculeComponents()` (`core/indigo-core/molecule/base_molecule.h#moleculeComponents`)
returns a `GraphDecomposer` run over the edges plus the atom sets of the haptic bonds of both types.
S-groups, SMARTS component groups and an attachment group no bond refers to join nothing. It is the
only source for the `indigo*Component*` functions, `indigoFragmentedSdf`, the `standardize` fragment
options and halide charges, and the wrappers' `checkSalt` / `stripSalt`. The wrappers also skip a
component that has haptic bonds: the salt patterns count ordinary bonds only, so to them the metal of
a complex is a lone ion. SMILES, InChI, substructure search, automapping, layout and the Bingo
cartridge keep the graph's components.

The molecule keeps the decomposer, the way the matchers keep theirs, and drops it when connectivity
changes, and only then. The graph reports every vertex and edge change through `changed()`; an edit of
a haptic bond or of the atoms of a group reports itself through
`BaseMolecule::_hapticConnectivityChanged()`. A reference taken before such a change does not outlive
it.

## Alternatives considered

- **Reuse the graph cache with haptic neighbours.** No new class, but the merged answer would stay in
  the caller's molecule and reach layout, InChI and the automapper depending on call order. The cache
  does not remember which sets it was computed with, and with an empty list — every molecule without
  haptic bonds — it never holds, so `componentIndex()` per atom would decompose the molecule on each
  call.
- **Make the graph's components follow haptic bonds.** Changes SMILES dots, InChI components and
  search semantics, all of which are defined on the graph (rejected already in the #3837 design).
- **Recompute per call without a cache.** `componentIndex()` per atom and the component iterators
  become quadratic.
- **Key the cache on the edit revision**, as `Molecule::_dativeModel()` is. Every edit moves the
  revision, a charge or a highlight included, so a loop that asks an atom for its component and then
  edits the atom decomposed the molecule at each step: 8.5 s against 0.07 s for 20,000 atoms.
- **A class of its own for the answer.** Written first: three arrays copied from the decomposer behind
  accessors that checked their arguments. `GraphDecomposer` holds the same three arrays and four
  matchers already keep one as a member, so the class was a second name for one thing.

## Consequences

- Invariant A7a: a component number belongs to the decomposition that produced it.
- Invariant A7b: a change of connectivity has to reach `BaseMolecule::changed()`.
- A new split the user sees goes through `moleculeComponents()`.
- Callers that work component by component report a complex as one entry: `calculate_molecule` and
  the SDF export of indigo-ketcher, `/calculate` of indigo-service, the component separator and the
  feature remover of the KNIME plugin. bingo-elastic hashes one record per component from
  `iterateComponents()`, so records with haptic bonds get new keys and need re-indexing; the Bingo
  cartridge's exact search stays on graph components, and the two engines differ on haptic structures
  by design.
- The API has no call left for the pieces that ordinary bonds alone hold together. A caller that
  wants them removes the haptic bonds from a copy first.
- `Graph`'s external-neighbour mode and `moleculeComponents()` do the same job on different sets, so
  a molecule has three decompositions until one mechanism takes over the other. Giving the mode's
  four users a decomposer of their own is the direction; it is not part of this change.
