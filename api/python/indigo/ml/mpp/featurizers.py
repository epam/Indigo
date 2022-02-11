import torch  # type: ignore

from indigo import Indigo, IndigoObject  # type: ignore

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)


def atomic_number(mol: IndigoObject) -> dict:
    """Get the atomic number for each atom (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of atomic numbers
    """
    atomic_numbers = []
    for atom in mol.iterateAtoms():
        atomic_numbers.append(atom.atomicNumber())

    return {"atomic": torch.tensor(atomic_numbers).unsqueeze(1)}


def atomic_degrees(mol: IndigoObject) -> dict:
    """Get the number of atoms direct neighbors (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of atomic degrees
    """
    degrees = []
    for atom in mol.iterateAtoms():
        degrees.append(atom.degree())

    return {"degrees": torch.tensor(degrees).unsqueeze(1)}


def atomic_isotopes(mol: IndigoObject) -> dict:
    """Get the isotope value for each atom (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of isotope values
    """
    isotopes = []
    for atom in mol.iterateAtoms():
        isotopes.append(atom.isotope())

    return {"isotopes": torch.tensor(isotopes).unsqueeze(1)}


def atomic_charges(mol: IndigoObject) -> dict:
    """Get the charge for each atom (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of atomic charges
    """
    charges = []
    for atom in mol.iterateAtoms():
        charges.append(atom.charge())

    return {"charges": torch.tensor(charges).unsqueeze(1)}


def atomic_valences(mol: IndigoObject) -> dict:
    """Get the valence for each atom (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of charges
    """
    valences = []
    for atom in mol.iterateAtoms():
        valences.append(atom.valence())

    return {"valences": torch.tensor(valences).unsqueeze(1)}


def atomic_radicals(mol: IndigoObject) -> dict:
    """Get the number of radical electrons for each atom (except implicit hydrogens) in a molecule.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor radical electrons numbers
    """
    radicals = []
    for atom in mol.iterateAtoms():
        radicals.append(atom.radicalElectrons())

    return {"radicals": torch.tensor(radicals).unsqueeze(1)}


def atom_in_ring(mol: IndigoObject) -> dict:
    """Get whether each atom (except implicit hydrogens) in a molecule is in aromatic ring.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of booleans
    """
    in_aromatic_ring = []
    for atom in mol.iterateAtoms():
        in_aromatic_ring.append(
            any(
                [
                    nei.bond().bondOrder() == 4
                    for nei in atom.iterateNeighbors()
                ]
            )
        )

    return {
        "in_aromatic_ring": torch.tensor(
            in_aromatic_ring, dtype=torch.int64
        ).unsqueeze(1)
    }


def stereocenter_types(mol: IndigoObject) -> dict:
    """Get stereocenter type for each atom (except implicit hydrogens) in a molecule.

    Possible values of stereocenter types are Indigo.ABS. Indigo.OR, Indigo.AND, Indigo.EITHER
    or zero if stereotype is unknown.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of stereocenter types for each atom

    """
    stereocenter_types = []
    for atom in mol.iterateAtoms():
        stereocenter_types.append(atom.stereocenterType())

    return {
        "stereocenter_types": torch.tensor(stereocenter_types).unsqueeze(1)
    }


def implicit_hydrogens(mol: IndigoObject) -> dict:
    """Get number of implicit hydrogens for each atom in a molecule

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of implicit hydrogens numbers
    """
    implicit_hydrogens = []
    for atom in mol.iterateAtoms():
        implicit_hydrogens.append(atom.countImplicitHydrogens())

    return {
        "implicit_hydrogens": torch.tensor(implicit_hydrogens).unsqueeze(1)
    }


def basic_pka_values(mol: IndigoObject, level=5, min_level=2) -> dict:
    """Get basic pka value for each atom in a molecule

    Args:
        IndigoObject: molecule object
        level: sets maximum level of advanced pKa model is used for pKa estimation
        min_level: sets minimal level of advanced pKa model is used for pKa estimation

    Returns:
        dict: key - feature name, value - torch.tensor of basic pka values
    """
    basic_pka_values = []
    for atom in mol.iterateAtoms():
        basic_pka_values.append(
            mol.getBasicPkaValue(atom, level=level, min_level=min_level)
        )

    return {"basic_pka_values": torch.tensor(basic_pka_values).unsqueeze(1)}


def acid_pka_values(mol: IndigoObject, level=5, min_level=2) -> dict:
    """Get acid pka value for each atom in a molecule

    Args:
        IndigoObject: molecule object
        level: sets maximum level of advanced pKa model is used for pKa estimation
        min_level: sets minimal level of advanced pKa model is used for pKa estimation

    Returns:
        dict: key - feature name, value - torch.tensor of acid pka values
    """
    acid_pka_values = []
    for atom in mol.iterateAtoms():
        acid_pka_values.append(
            mol.getAcidPkaValue(atom, level=level, min_level=min_level)
        )

    return {"acid_pka_values": torch.tensor(acid_pka_values).unsqueeze(1)}


def atomic_masses(mol: IndigoObject) -> dict:
    """Get acid pka value for each atom in a molecule

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of acid pka values
    """

    atomic_masses = []
    indigo = Indigo()

    for atom in mol.iterateAtoms():
        # hack way of obtaining atomic mass, should be reimplement afterwards
        smiles = fr"[{atom.isotope()}{atom.symbol()}]"
        atom_molecule = indigo.loadMolecule(smiles)
        atomic_masses.append(round(atom_molecule.molecularWeight(), 4))

    return {"atomic_masses": torch.tensor(atomic_masses).unsqueeze(1)}


def bond_order(mol: IndigoObject) -> dict:
    """Get the bond order for each bond in a molecule.

    Possible values of bond orders are 1 (single), 2 (double), 3 (triple), 4 (aromatic) or 0 (bond is ambiguous)

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of bond orders
    """
    orders = []
    for bond in mol.iterateBonds():
        orders.append(bond.bondOrder())

    return {"orders": torch.tensor(orders * 2).unsqueeze(1).float()}


def topologies(mol: IndigoObject) -> dict:
    """Get whether bonds in a molecule are in a ring or in a chain.

    Possible values are Indigo.RING, Indigo.CHAIN or 0 (bond is ambiguous)


    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of bonds topologies
    """
    topologies = []
    for bond in mol.iterateBonds():
        topologies.append(bond.topology())

    return {"topologies": torch.tensor(topologies * 2).unsqueeze(1).float()}


def aromatic_bonds(mol: IndigoObject) -> dict:
    """Get whether bonds in a molecule are aromatic or not.

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of booleans
    """
    is_aromatic = []
    for bond in mol.iterateBonds():
        is_aromatic.append(bond.bondOrder() == 4)

    return {"is_aromatic": torch.tensor(is_aromatic * 2).unsqueeze(1).float()}


def bond_stereo(mol: IndigoObject) -> dict:
    """Get stereotype of each bond in a molecule.

    Possible values are Indigo.UP ("up" bond), Indigo.DOWN ("down" bond), Indigo.EITHER ("either" bond), Indigo.CIS ("cis" double bond). Indigo.TRANS ("trans" double bond)
    or zero if not a stereo bond of any kind

    Args:
        IndigoObject: molecule object

    Returns:
        dict: key - feature name, value - torch.tensor of stereotypes for each bond
    """
    bond_stereo = []
    for bond in mol.iterateBonds():
        bond_stereo.append(bond.bondStereo())

    return {"bond_stereo": torch.tensor(bond_stereo * 2).unsqueeze(1).float()}
