import pytest

from indigo import Indigo

from indigo.ml.hybridization import get_hybridization


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (
            "c1ccccc1",  # benzene
            [
                "sp2",
                "sp2",
                "sp2",
                "sp2",
                "sp2",
                "sp2",
                "unhybridized",
                "unhybridized",
                "unhybridized",
                "unhybridized",
                "unhybridized",
                "unhybridized",
            ],
        ),
        ("[C-]#[O+]", ["sp", "sp"]),  # carbon monoxide
        ("O=C=O", ["sp2", "sp", "sp2"]),  # carbon dioxide
        ("C#N", ["sp", "sp", "unhybridized"]),  # hydrogen cyanide
        (
            "O=C(N)C",  # acetamide
            [
                "sp2",
                "sp2",
                "sp2",
                "sp3",
                "unhybridized",
                "unhybridized",
                "unhybridized",
                "unhybridized",
                "unhybridized",
            ],
        ),
    ],
)
def test_get_hybridization(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    mol.unfoldHydrogens()
    mol.aromatize()

    hybridizations = []
    for atom in mol.iterateAtoms():
        hybridization = get_hybridization(atom)
        hybridizations.append(hybridization)
    assert hybridizations == expecting
