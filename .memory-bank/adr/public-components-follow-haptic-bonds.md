# Public components follow haptic bonds; graph algorithms keep the graph's components

**Date:** 2026-09-30 · **Status:** Accepted · **Ticket:** #3927

## Context

A haptic bond is not an edge ([haptic-bond-endpoints.md](./haptic-bond-endpoints.md)), so the graph
splits a ferrocene into two rings and an iron. The public component API reported that split, while
KET — through `collectExternalNeighbors` — already wrote the complex as one molecule node; the
fragment options of `standardize` and the wrappers' salt helpers removed the metal of a complex.

The graph's own component cache serves two modes through one set of readers: after
`countComponents(external_neighbors)` its plain readers (`getDecomposition()`, `vertexComponent()`)
return the merged decomposition until the next edit, and `addVertex()` does not invalidate it.
Layout (`MoleculeLayout::_atomToAtomHapticRings`), the layered code and the automapper read those
plain readers on the caller's molecule.

## Decision

`BaseMolecule::moleculeComponents()` (`core/indigo-core/molecule/base_molecule.h#moleculeComponents`)
— edges plus haptic bonds of both types, not S-groups and not SMARTS component groups, cached by edit
revision — is the only source for the `indigo*Component*` functions, `indigoFragmentedSdf`, the
`standardize` fragment options and halide charges, and the wrappers' `checkSalt` / `stripSalt`.
SMILES, InChI, substructure search, automapping, layout and the Bingo cartridge keep the graph's
components.

## Alternatives considered

- **Reuse the graph cache with haptic neighbours.** No new class, but the merged answer stays in the
  caller's molecule and reaches layout and the layered code depending on call order; the cache is
  stale after `addVertex()`, and its edge count counts every pair of a neighbour set as an edge.
- **Make the graph's components follow haptic bonds.** Changes SMILES dots, InChI components and
  search semantics, all of which are defined on the graph (rejected already in the #3837 design).
- **Recompute per call without a cache.** `componentIndex()` per atom and the component iterators
  become quadratic.

## Consequences

- Invariant A7a: a component index belongs to the decomposition that produced it.
- A new split the user sees goes through `moleculeComponents()`.
- bingo-elastic hashes one record per component from `iterateComponents()`, so records with haptic
  bonds get new keys and need re-indexing; the Bingo cartridge's exact search stays on graph
  components, and the two engines differ on haptic structures by design.
- The cache is rebuilt after any edit, coordinates and highlighting included, not only topology.
