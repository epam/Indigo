import pytest

from indigo import Indigo
from indigo.hybridization import get_hybridization


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (
            "c1ccccc1",  # benzene
            [
                "SP2",
                "SP2",
                "SP2",
                "SP2",
                "SP2",
                "SP2",
            ],
        ),
        (
            "OC1=CC=CC=C1",  # phenol
            ["SP2", "SP2", "SP2", "SP2", "SP2", "SP2", "SP2"],
        ),
        ("[C-]#[O+]", ["SP", "SP"]),  # carbon monoxide
        ("O=C=O", ["SP2", "SP", "SP2"]),  # carbon dioxide
        ("C#N", ["SP", "SP"]),  # hydrogen cyanide
        (
            "O=C(N)C",  # acetamide
            [
                "SP2",
                "SP2",
                "SP",
                "SP3",
            ],
        ),
        (
            "OS(=O)(=O)O",  # sulfuric acid
            [
                "SP3",
                "SP3",
                "SP2",
                "SP2",
                "SP3",
            ],
        ),
        ("N(=O)O", ["SP2", "SP2", "SP3"]),  # nitrous acid
        (
            "O=[Xe](=O)(=O)=O",  # xenon tetroxide
            ["SP2", "SP3", "SP2", "SP2", "SP2"],
        ),
        (
            "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
            ["SP3", "SP3D2", "SP3", "SP3", "SP3", "SP3", "SP3"],
        ),
        ("FBr(F)F", ["SP3", "SP3D", "SP3", "SP3"]),  # bromine trifluoride
        ("[Be](Cl)Cl", ["SP", "SP3", "SP3"]),
        ("C1=CC=CS1", ["SP2", "SP2", "SP2", "SP2", "SP2"]),  # thiophene
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
