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
        (
            "OS(=O)(=O)O",  # sulfuric acid
            [
                "sp3",
                "sp3",
                "sp2",
                "sp2",
                "sp3",
                "unhybridized",
                "unhybridized",
            ],
        ),
        ("N(=O)O", ["sp2", "sp2", "sp3", "unhybridized"]),  # nitrous acid
        (
            "O=[Xe](=O)(=O)=O",  # xenon tetroxide
            ["sp2", "sp3", "sp2", "sp2", "sp2"],
        ),
        (
            "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
            ["sp3", "sp3d2", "sp3", "sp3", "sp3", "sp3", "sp3"],
        ),
        ("FBr(F)F", ["sp3", "sp3d", "sp3", "sp3"]),  # bromine trifluoride
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
