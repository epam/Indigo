import pytest

from indigo import Indigo
from indigo.hybridization import get_hybridization


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
            ],
        ),
        (
            "OC1=CC=CC=C1",  # phenol
            ["sp2", "sp2", "sp2", "sp2", "sp2", "sp2", "sp2"],
        ),
        ("[C-]#[O+]", ["sp", "sp"]),  # carbon monoxide
        ("O=C=O", ["sp2", "sp", "sp2"]),  # carbon dioxide
        ("C#N", ["sp", "sp"]),  # hydrogen cyanide
        (
            "O=C(N)C",  # acetamide
            [
                "sp2",
                "sp2",
                "sp",
                "sp3",
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
            ],
        ),
        ("N(=O)O", ["sp2", "sp2", "sp3"]),  # nitrous acid
        (
            "O=[Xe](=O)(=O)=O",  # xenon tetroxide
            ["sp2", "sp3", "sp2", "sp2", "sp2"],
        ),
        (
            "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
            ["sp3", "sp3d2", "sp3", "sp3", "sp3", "sp3", "sp3"],
        ),
        ("FBr(F)F", ["sp3", "sp3d", "sp3", "sp3"]),  # bromine trifluoride
        ("[Be](Cl)Cl", ["sp", "sp3", "sp3"]),
        ("C1=CNC=C1", ["sp2", "sp2", "sp2", "sp2", "sp2"]),
        ("C1=CC=CS1", ["sp2", "sp2", "sp2", "sp2", "sp2"]),  # thiophene
    ],
)
def test_get_hybridization(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)

    hybridizations = []
    for atom in mol.iterateAtoms():
        hybridization = get_hybridization(atom)
        hybridizations.append(hybridization)
    assert hybridizations == expecting
