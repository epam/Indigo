from indigo import Indigo  # type: ignore

indigo = Indigo()
mol = indigo.loadMolecule("c1ccccc1")  # benzene
# mol = indigo.loadMolecule("[C-]#[O+]")  # carbon monoxide
# mol = indigo.loadMolecule("O=C=O")  # carbon dioxide
# mol = indigo.loadMolecule("OC1=CC=CC=C1")  # phenol
# mol = indigo.loadMolecule("Cl")  # HCl
# mol = indigo.loadMolecule("C")  # methan

mol.unfoldHydrogens()
mol.aromatize()

_hybridization_for_orbs = {
    1: "s",
    2: "sp",
    3: "sp2",
    4: "sp3"
}


def num_bonds(atom):
    bonds = 0
    for nei in atom.iterateNeighbors():
        print(nei.symbol(), nei.bond().bondOrder())
        bonds += nei.bond().bondOrder()
    print(f"Atom {atom.symbol()} has {bonds} bonds")
    return bonds


# def lone_pairs(atom, n_bonds):
#     if n_bonds == 1:
#         return 0
#     atomic_number = atom.atomicNumber()
#     if atomic_number <= 4:
#         return 0
#     elif atomic_number <= 10:
#         if n_bonds == atomic_number - 2:
#             return 0
#         return atomic_number - 2 - n_bonds
#     elif atomic_number <= 12:
#         return 0
#     elif atomic_number <= 18:
#         if n_bonds == atomic_number - 2:
#             return 0
#         return atomic_number - 2 - n_bonds


def cardon_hybridization(carbon):
    neighbors = len([1 for _ in carbon.iterateNeighbors()])
    if neighbors <= 2:
        return "sp"
    if neighbors == 4:
        return "sp3"
    if neighbors == 3:
        return "sp2"
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
        if match_minus_induction(nei):
            return "sp2"
    return "sp3"


def nitrogen_hybridization(nitrogen):
    doubles = 0
    minus_induction = False
    for nei in nitrogen.iterateNeighbors():
        if nei.bond().bondOrder() == 3:
            return "sp"
        if nei.bond().bondOrder() == 2:
            doubles += 1
        if match_minus_induction(nei):
            minus_induction = True
    if doubles > 1 or minus_induction:
        return "sp2"
    return "sp3"


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

    # number of hybridized orbitals for central atom = (number of outer shell
    # electrons plus number of monovalent neighbors minus cation charge plus
    # anion charge) / 2
    # H = (O + M - C + A) / 2
    n_orbs = 1  # waiting for atom.outer_electrons method
    return _hybridization_for_orbs[n_orbs]


# hybridizations = []
# for atom in mol.iterateAtoms():
#     print(f"\nAtom {atom.index()} {atom.symbol()}: {atom.degree()} neighbors")
#     hybridization = get_hybridization(atom)
#     print(f"Atom {atom.symbol()} has {hybridization} hybridization")
#     hybridizations.append(hybridization)
#
# print(hybridizations)
