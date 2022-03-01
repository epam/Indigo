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
from typing import Collection

from indigo import IndigoObject
from indigo.hybridization import HybridizationType

ALIPHATIC_CYCLES = tuple("C1" + "C" * n + "C1" for n in range(1, 12))
ALIPHATIC_HETEROCYCLES = tuple(
    ["C1N" + "C" * n + "C1" for n in range(5)]
    + ["C1O" + "C" * n + "C1" for n in range(5)]
    + ["C1S" + "C" * n + "C1" for n in range(5)]
)
AROMATIC_CYCLES = (
    "C1C=C1",
    "C1=CC=C1",
    "C1C=CC=C1",
    "C1=CC=CC=C1",
    "C1C=CC=CC=C1",
    "C1=CC=CC=CC=C1",
)


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


def count_matches(molecule: IndigoObject, cycles: Collection) -> int:
    matcher = molecule.dispatcher.substructureMatcher(molecule)
    matches = 0
    for cycle in cycles:
        query = molecule.dispatcher.loadQueryMolecule(cycle)
        matches += matcher.countMatches(query)
        print(cycle, matches)
    return matches


def n_aliphatic_cycles(molecule: IndigoObject) -> int:
    """Match and count aliphatic cycles in molecule.

    Match all aliphatic cycles from cyclopropane to cyclotridecane (C3-C13)"""
    return count_matches(molecule, ALIPHATIC_CYCLES)


def n_aliphatic_heterocycles(molecule: IndigoObject) -> int:
    """Match and count aliphatic heterocycles in molecule"""
    return count_matches(molecule, ALIPHATIC_HETEROCYCLES)


def n_aromatic_cycles(molecule: IndigoObject) -> int:
    """Match and count aromatic cycles in molecule"""
    return count_matches(molecule, AROMATIC_CYCLES)
