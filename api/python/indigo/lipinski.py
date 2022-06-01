"""Lipinski's rule states that, in general, an orally active drug has no more
than one violation of the following criteria:
- No more than 5 hydrogen bond donors (the total number of nitrogen–hydrogen
and oxygen–hydrogen bonds)
- No more than 10 hydrogen bond acceptors (all nitrogen or oxygen atoms)
- A molecular mass less than 500 daltons (a.m.u)
- An octanol-water partition coefficient (log P) that does not exceed 5

Note that all numbers are multiples of five, which is the origin of the rule's
name.
"""
from indigo import IndigoObject
from indigo.hybridization import HybridizationType, in_aromatic_ring


def n_hydrogen_donors(molecule: IndigoObject) -> int:
    """The total number of nitrogen-hydrogen and oxygen-hydrogen bonds."""
    hydrogen_donors = 0
    for atom in molecule.iterateAtoms():
        if atom.symbol() == "N" or atom.symbol() == "O":
            hydrogen_donors += atom.countHydrogens()
    return hydrogen_donors


def n_hydrogen_acceptors(molecule: IndigoObject) -> int:
    """The total number of nitrogen and oxygen atoms."""
    return sum(
        [
            atom.symbol() == "N" or atom.symbol() == "O"
            for atom in molecule.iterateAtoms()
        ]
    )


def lipinski_criteria(molecule: IndigoObject) -> bool:
    """Check if compound satisfies Lipinski's rule of five."""
    violations = 0
    if n_hydrogen_donors(molecule) > 5:
        violations += 1
    if n_hydrogen_acceptors(molecule) > 10:
        violations += 1
    if molecule.molecularWeight() > 500:
        violations += 1
    if molecule.logP() > 5:
        violations += 1
    return violations <= 1


def sp3_carbon_ratio(molecule: IndigoObject) -> float:
    """Ratio of SP3 hybridized carbons

    Return the value between 0 and 1: the ratio of sp3‐hybridized carbons to
    the total number of carbons.
    """
    carbons = 0
    sp3_carbons = 0
    for atom in molecule.iterateAtoms():
        if atom.symbol() == "C":
            carbons += 1
            if atom.getHybridization() == HybridizationType.SP3:
                sp3_carbons += 1
    if carbons == 0:
        return 0
    return round(sp3_carbons / carbons, 3)


def atom_indices_in_rings(molecule: IndigoObject) -> list[set[int]]:
    """Return list with sets of atom indices in every ring.

    Sometimes iterateRings return the number of rings greater than it actually
    is (e.g. bicyclobutane). To solve this superrings list is used. If the ring
    contains the whole another ring (all it's atoms) it is detected as a
    superring and will be skipped.
    """
    rings_indices = []
    for ring in molecule.iterateRings(
        min_atoms=3, max_atoms=molecule.countAtoms()
    ):  # default value? 6?
        rings_indices.append(set(atom.index() for atom in ring.iterateAtoms()))
    rings_indices.sort()

    superrings = []
    for i, ring in enumerate(rings_indices):
        for r in rings_indices[i + 1 :]:
            if r in superrings:
                continue
            if r.issuperset(ring):
                superrings.append(r)
    return [ring for ring in rings_indices if ring not in superrings]


def n_aliphatic_cycles(molecule: IndigoObject) -> int:
    """Count aliphatic (not aromatic) cycles in molecule.

    Molecule should be aromatized.
    """
    aliphatic_cycles = 0
    rings_with_indices = atom_indices_in_rings(molecule)
    for ring in rings_with_indices:
        atoms = [molecule.getAtom(atom_idx) for atom_idx in ring]
        print([not in_aromatic_ring(atom) for atom in atoms])
        if all([not in_aromatic_ring(atom) for atom in atoms]) and all(
            [atom.symbol() == "C" for atom in atoms]
        ):
            aliphatic_cycles += 1
    return aliphatic_cycles


def n_aliphatic_heterocycles(molecule: IndigoObject) -> int:
    """Count aliphatic heterocycles in molecule.

    Molecule should be aromatized.
    """
    aliphatic_heterocycles = 0
    rings_with_indices = atom_indices_in_rings(molecule)
    for ring in rings_with_indices:
        atoms = [molecule.getAtom(atom_idx) for atom_idx in ring]
        atom_symbols = [atom.symbol() for atom in atoms]
        print([not in_aromatic_ring(atom) for atom in atoms])
        if all([not in_aromatic_ring(atom) for atom in atoms]) and any(
            [
                "S" in atom_symbols,
                "O" in atom_symbols,
                "N" in atom_symbols,
                "P" in atom_symbols,
                "B" in atom_symbols,
            ]
        ):
            aliphatic_heterocycles += 1
    return aliphatic_heterocycles


def n_aromatic_cycles(molecule: IndigoObject) -> int:
    """Count aromatic cycles in molecule.

    Molecule should be aromatized.
    """
    aromatic_cycles = 0
    rings_with_indices = atom_indices_in_rings(molecule)
    for ring in rings_with_indices:
        atoms = [molecule.getAtom(atom_idx) for atom_idx in ring]
        print([in_aromatic_ring(atom) for atom in atoms])
        if all([in_aromatic_ring(atom) for atom in atoms]):
            aromatic_cycles += 1
    return aromatic_cycles
