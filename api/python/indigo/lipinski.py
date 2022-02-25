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
    """Checks if compound satisfies Lipinski's rule of five."""
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
