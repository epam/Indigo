import math

from indigo import Indigo  # type: ignore

indigo = Indigo()

mol = indigo.loadMolecule("N.N.N.N.N.N.[Cl-].[Cl-].[Co+2]")

mol.unfoldHydrogens()
mol.aromatize()


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
    "V": 5,
    "Cr": 6,  # 4s1 3d5 (!)
    "Mn": 7,  # 4s2 3d5
    "Fe": 8,  # 4s2 3d6
    "Co": 9,
    "Ni": 10,
    "Cu": 11,
    "Zn": 12,
    "Ga": 13,
    "Ge": 14,
    "As": 15,
}

N_ORBS_HYBRIDIZATION = {1: "unhybridized", 2: "sp", 3: "sp2", 4: "sp3"}


def num_bonds(atom):
    bonds = 0
    for nei in atom.iterateNeighbors():
        # print(nei.symbol(), nei.bond().bondOrder())
        bonds += nei.bond().bondOrder()
    # print(f"Atom {atom.symbol()} has {bonds} bonds")
    return bonds


def has_lone_pair(atom):
    return OUTER_ELECTRONS[atom.symbol()] - atom.degree() >= 2


def lone_pairs(atom, n_bonds):
    outer_electrons = OUTER_ELECTRONS[atom.symbol()]
    pairs = math.floor((outer_electrons - atom.degree()) / 2)
    print(pairs)
    return pairs


def cardon_hybridization(carbon):
    neighbors = carbon.degree()
    if neighbors == 4:
        return "sp3"
    if neighbors == 3:
        return "sp2"
    if neighbors <= 2:
        return "sp"
    return None


def match_minus_induction(atom):
    for nei in atom.iterateNeighbors():
        if nei.bond().bondOrder() in [2, 4]:
            return True
    return False


def oxygen_hybridization(oxygen):
    for nei in oxygen.iterateNeighbors():
        if nei.bond().bondOrder() == 2:
            return "sp2"
        if nei.bond().bondOrder() == 3:
            # for CO, but some sources do not recognize "sp" for oxygen
            return "sp"
        if nei.symbol() == "C" and match_minus_induction(nei):
            return "sp2"
    return "sp3"


def nitrogen_hybridization(nitrogen):
    n_orbs = nitrogen.degree() + has_lone_pair(nitrogen)
    minus_induction = False
    for nei in nitrogen.iterateNeighbors():
        if nei.symbol() == "C" and match_minus_induction(nei):
            minus_induction = True
    if n_orbs == 4 and minus_induction:
        return N_ORBS_HYBRIDIZATION[n_orbs - 1]
    return N_ORBS_HYBRIDIZATION[n_orbs]


def complex_hybridization(atom, neighbors):
    if neighbors == 4:
        if atom.symbol() in ["Pt", "Ni", "Cu", "Au", "Pd"]:  # else?
            return "sp2d"
        return "sp3"
    if neighbors == 5:
        return "sp3d"
    if neighbors == 6:
        return "sp3d2"
    if neighbors == 7:
        return "sp3d3"
    if neighbors == 8:
        return "sp3d4"
    return None


def get_hybridization(atom):
    # if the atomic number is undefined or ambiguous
    if atom.atomicNumber() == 0:
        return None

    # don't bother with the actinides and beyond
    elif atom.atomicNumber() >= 89:
        return "unhybridized"  # or "s"?

    n_bonds = num_bonds(atom)

    if n_bonds <= 1:
        return "unhybridized"  # or "s"?
    if n_bonds >= 8:  # aromatic
        return "sp2"

    if atom.atomicNumber() == 6:
        return cardon_hybridization(atom)
    if atom.atomicNumber() == 8:
        return oxygen_hybridization(atom)
    if atom.atomicNumber() == 7:
        return nitrogen_hybridization(atom)

    if atom.degree() >= 4:
        complex_hybridization(atom, atom.degree())

    # number of hybridized orbitals for central atom = (number of outer shell
    # electrons plus number of monovalent neighbors minus cation charge plus
    # anion charge) / 2
    # H = (O + M - C + A) / 2

    # a simpler formula:
    # H = O + L
    # number of neighbours + number of lobe electron pairs

    n_orbs = atom.degree() + lone_pairs
    return N_ORBS_HYBRIDIZATION[n_orbs]


# hybridizations = []
# for atom in mol.iterateAtoms():
#     print(f"\nAtom {atom.index()} {atom.symbol()}: {atom.degree()} neighbors")
#     hybridization = get_hybridization(atom)
#     print(f"Atom {atom.symbol()} has {hybridization} hybridization")
#     hybridizations.append(hybridization)
#
# print(hybridizations)
