import pytest

from indigo import Indigo
from indigo.hybridization import get_hybridization, EnumHybridizations


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (
            "c1ccccc1",  # benzene
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
            ],
        ),
        (
            "OC1=CC=CC=C1",  # phenol
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
            ],
        ),
        (
            "[C-]#[O+]",
            [EnumHybridizations.SP, EnumHybridizations.SP],
        ),  # carbon monoxide
        (
            "O=C=O",
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP,
                EnumHybridizations.SP2,
            ],
        ),  # carbon dioxide
        (
            "C#N",
            [EnumHybridizations.SP, EnumHybridizations.SP],
        ),  # hydrogen cyanide
        (
            "O=C(N)C",  # acetamide
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP,
                EnumHybridizations.SP3,
            ],
        ),
        (
            "OS(=O)(=O)O",  # sulfuric acid
            [
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP3,
            ],
        ),
        (
            "N(=O)O",
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP3,
            ],
        ),  # nitrous acid
        (
            "O=[Xe](=O)(=O)=O",  # xenon tetroxide
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP3,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
            ],
        ),
        (
            "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
            [
                EnumHybridizations.SP3,
                EnumHybridizations.SP3D2,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
            ],
        ),
        (
            "FBr(F)F",
            [
                EnumHybridizations.SP3,
                EnumHybridizations.SP3D,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
            ],
        ),  # bromine trifluoride
        (
            "[Be](Cl)Cl",
            [
                EnumHybridizations.SP,
                EnumHybridizations.SP3,
                EnumHybridizations.SP3,
            ],
        ),
        (
            "C1=CC=CS1",
            [
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
                EnumHybridizations.SP2,
            ],
        ),  # thiophene
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
