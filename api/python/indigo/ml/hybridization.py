from indigo import Indigo  # type: ignore

indigo = Indigo()
# mol = indigo.loadMolecule("c1ccccc1")  # benzene
mol = indigo.loadMolecule("[C-]#[O+]")  # carbon monoxide


for atom in mol.iterateAtoms():
    print(f"\nAtom {atom.index()} {atom.symbol()}: {atom.degree()} neighbors")
    bonds = 0
    for nei in atom.iterateNeighbors():
        print(
            f"neighbor atom {nei.symbol()} is connected by {nei.bond().index()}"
            f" bond"
        )
        bonds += nei.bond().bondOrder()
    print(f"Atom {atom.symbol()} has {bonds} bonds")


def num_bonds(atom):
    bonds = 0
    for nei in atom.iterateNeighbors():
        bonds += nei.bond().bondOrder()
    return bonds


# def val_electrons(atom, n_bonds):
#     if n_bonds == 1:
#         return 1
#     atomic_number = atom.atomicNumber()
#     if atomic_number <= 2:
#         return atomic_number
#     elif atomic_number <= 10:
#         if n_bonds == atomic_number - 2:
#             return n_bonds
#         return atomic_number - 2
#     elif atomic_number <= 18:
#         return atomic_number - 10


def lone_pairs(atom, n_bonds):
    if n_bonds == 1:
        return 0
    atomic_number = atom.atomicNumber()
    if atomic_number <= 4:
        return 0
    elif atomic_number <= 10:
        if n_bonds == atomic_number - 2:
            return 0
        return atomic_number - 2 - n_bonds
    elif atomic_number <= 12:
        return 0
    elif atomic_number <= 18:
        if n_bonds == atomic_number - 2:
            return 0
        return atomic_number - 2 - n_bonds


def get_hybridization(atom):
    # if the atomic number is undefined or ambiguous
    if atom.atomicNumber() == 0:
        return None

    # don't bother with the actinides and beyond
    elif atom.atomicNumber() >= 89:
        return "s"  # or "unhybridized"?

    n_bonds = num_bonds(atom)
    if n_bonds >= 8:
        return "sp2"  # aromatic

    # number of hybridized orbitals = number of bonded atoms plus number of
    # lone pair electrons
    n_orbs = n_bonds + lone_pairs(atom, n_bonds)
