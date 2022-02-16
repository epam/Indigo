import math
from enum import Enum
from typing import TYPE_CHECKING

from indigo.exceptions import IndigoException

if TYPE_CHECKING:
    from indigo import IndigoObject

OUTER_ELECTRONS = {
    "H": 1,
    "He": 2,
    "Li": 1,
    "Be": 2,
    "B": 3,
    "C": 4,
    "N": 5,
    "O": 6,
    "F": 7,
    "Ne": 8,
    "Na": 1,
    "Mg": 2,
    "Al": 3,
    "Si": 4,
    "P": 5,
    "S": 6,
    "Cl": 7,
    "Ar": 8,
    "K": 1,
    "Ca": 2,
    "Sc": 3,
    "Ti": 4,
    "V": 5,  # 4s2 3d3 (!)
    "Cr": 6,  # 4s1 3d5 (!)
    "Mn": 7,  # 4s2 3d5
    "Fe": 8,  # 4s2 3d6
    "Co": 9,
    "Ni": 10,
    "Cu": 1,
    "Zn": 2,
    "Ga": 3,
    "Ge": 4,
    "As": 5,
    "Se": 6,
    "Br": 7,
    "Kr": 8,
    "Rb": 1,
    "Sr": 2,
    "Y": 3,  # 5s2 4d1
    "Zr": 4,  # 5s2 4d2
    "Nb": 5,  # 5s1 4d4 (!)
    "Mo": 6,  # 5s1 4d5
    "Tc": 7,  # 5s2 4d5
    "Ru": 8,  # 5s1 4d7 (!!!)
    "Rh": 9,  # 5s1 4d8
    "Pd": 10,  # 4d10 (!)
    "Ag": 1,
    "Cd": 2,
    "In": 3,
    "Sn": 4,
    "Sb": 5,
    "Te": 6,
    "I": 7,
    "Xe": 8,
    "Cs": 1,
    "Ba": 2,
    "La": 3,  # 6s2 5d1
    "Ce": 4,  # 6s2 4f2 (!!!) Atomic num = 58, lantanoids
    "Hf": 4,
    "Ta": 5,
    "W": 6,
    "Re": 7,
    "Os": 8,
    "Ir": 9,
    "Pt": 10,
    "Au": 11,
    "Hg": 2,
    "Tl": 3,
    "Pb": 4,
    "Bi": 5,
    "Po": 6,
    "At": 7,
    "Rn": 8,
    "Fr": 1,
    "Ra": 2,
}


class HybridizationType(Enum):
    S = 1
    SP = 2
    SP2 = 3
    SP3 = 4
    SP3D = 5
    SP3D2 = 6
    SP3D3 = 7
    SP3D4 = 8
    SP2D = 9


def num_bonds(atom: "IndigoObject") -> int:
    bonds = 0
    for nei in atom.iterateNeighbors():
        bonds += nei.bond().bondOrder()
    bonds += atom.countImplicitHydrogens()
    return bonds


def has_lone_pair(atom: "IndigoObject", n_bonds: int) -> bool:
    return OUTER_ELECTRONS[atom.symbol()] - n_bonds >= 2


def lone_pairs(atom: "IndigoObject", n_bonds: int) -> int:
    outer_electrons = OUTER_ELECTRONS[atom.symbol()]
    pairs = math.floor((outer_electrons - n_bonds) / 2)
    return pairs


def carbon_hybridization(carbon: "IndigoObject") -> HybridizationType:
    neighbors = carbon.degree() + carbon.countImplicitHydrogens()
    if neighbors == 4:
        return HybridizationType.SP3
    if neighbors == 3:
        return HybridizationType.SP2
    if neighbors <= 2:
        return HybridizationType.SP
    raise IndigoException("Couldn't calculate C hybridization properly")


def match_minus_induction(atom: "IndigoObject") -> bool:
    for nei in atom.iterateNeighbors():
        if nei.bond().bondOrder() == 2:
            return True
    return False


def oxygen_hybridization(oxygen: "IndigoObject") -> HybridizationType:
    for nei in oxygen.iterateNeighbors():
        if nei.bond().bondOrder() == 2:
            return HybridizationType.SP2
        if nei.bond().bondOrder() == 3:
            # for CO, but some sources do not recognize "sp" for oxygen
            return HybridizationType.SP
        if nei.symbol() == "C" and match_minus_induction(nei):
            return HybridizationType.SP2
    return HybridizationType.SP3


def nitrogen_hybridization(
    nitrogen: "IndigoObject", n_bonds: int
) -> HybridizationType:
    n_orbs = nitrogen.degree() + has_lone_pair(nitrogen, n_bonds)
    minus_induction = False
    for nei in nitrogen.iterateNeighbors():
        if nei.symbol() == "C" and match_minus_induction(nei):
            minus_induction = True
    if n_orbs == 4 and minus_induction:
        return HybridizationType(n_orbs - 1)
    if n_orbs <= 4:
        return HybridizationType(n_orbs)
    raise IndigoException("Couldn't calculate N hybridization properly")


def complex_hybridization(
    atom: "IndigoObject", neighbors: int
) -> HybridizationType:
    if neighbors == 4:
        if atom.symbol() in ["Pt", "Ni", "Cu", "Au", "Pd"]:
            return HybridizationType.SP2D
        return HybridizationType.SP3
    if neighbors == 5:
        return HybridizationType.SP3D
    if neighbors == 6:
        return HybridizationType.SP3D2
    if neighbors == 7:
        return HybridizationType.SP3D3
    if neighbors == 8:
        return HybridizationType.SP3D4
    raise IndigoException(
        f"Couldn't calculate {atom.symbol()} hybridization properly"
    )


def in_aromatic_ring(atom: "IndigoObject") -> bool:
    for nei in atom.iterateNeighbors():
        if nei.bond().topology() == 10:
            return True
    return False


def get_hybridization(atom: "IndigoObject") -> HybridizationType:
    """Returns HybridizationType for an atom from a molecule.

    Works only with atoms with atomic numbers from 1 to 56. Don't bother with
    the lantanoids and beyond. If atomic number is undefined or ambiguous or >
    56 raises IndigoException.

    Args:
        atom: an indigo.IndigoObject for the atom.
    Returns:
        HybridizationType: atom hybridization. Could be HybridizationType.S
        for unhybridized atom and HybridizationType.SP, HybridizationType.SP2,
        etc. for hybridized.
    """
    # if the atomic number is undefined or ambiguous
    if atom.atomicNumber() == 0:
        raise IndigoException("Atomic number is undefined or ambiguous")

    # don't bother with the lanthanides, actinides and beyond
    if 57 <= atom.atomicNumber() <= 71 or atom.atomicNumber() >= 89:
        raise IndigoException(
            "Hybridization calculation is not implemented for lanthanides, "
            "actinides and beyond."
        )
    if atom.atomicNumber() == 1:
        return HybridizationType.S

    n_bonds = num_bonds(atom)

    if n_bonds == 0:
        return HybridizationType.S

    if atom.symbol() in ["C", "N", "O", "P", "S", "B"] and in_aromatic_ring(
        atom
    ):
        return HybridizationType.SP2

    if atom.atomicNumber() == 6:
        return carbon_hybridization(atom)
    if atom.atomicNumber() == 8:
        return oxygen_hybridization(atom)
    if atom.atomicNumber() == 7:
        return nitrogen_hybridization(atom, n_bonds)

    if atom.degree() >= 4:
        return complex_hybridization(atom, atom.degree())

    # number of hybridized orbitals = number of neighbours + number of lone
    # electron pairs
    # H = N + L

    n_orbs = atom.degree() + lone_pairs(atom, n_bonds)
    return HybridizationType(n_orbs)
