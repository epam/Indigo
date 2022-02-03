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
            f"bond"
        )
        bonds += nei.bond().bondOrder()
    print(f"Atom {atom.symbol()} has {bonds} bonds")


def num_bonds(atom):
    bonds = 0
    for nei in atom.iterateNeighbors():
        bonds += nei.bond().bondOrder()
    return bonds


def lone_pairs(atom):
    pass


def get_hybridization(atom):
    if atom.atomicNumber() == 0:  # if the atomic number is undefined or ambiguous
        return None
    elif atom.atomicNumber() >= 89:
        # don't bother with the actinides and beyond
        return "s"  # or "unhybridized"?

    n_bonds = num_bonds(atom)
    if bonds >= 8:
        return "sp2"  # aromatic

    n_orbs = n_bonds + lone_pairs(atom)
    # number of hybridized orbitals = number of bonded atoms plus number of
    # lone pair electrons
