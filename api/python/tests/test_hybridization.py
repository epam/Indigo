import pytest

from indigo import Indigo
from indigo.hybridization import HybridizationType, get_hybridization


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (
            "c1ccccc1",  # benzene
            [
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
            ],
        ),
        (
            "OC1=CC=CC=C1",  # phenol
            [
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
            ],
        ),
        (
            "[C-]#[O+]",
            [HybridizationType.SP, HybridizationType.SP],
        ),  # carbon monoxide
        (
            "O=C=O",
            [
                HybridizationType.SP2,
                HybridizationType.SP,
                HybridizationType.SP2,
            ],
        ),  # carbon dioxide
        (
            "C#N",
            [HybridizationType.SP, HybridizationType.SP],
        ),  # hydrogen cyanide
        (
            "O=C(N)C",  # acetamide
            [
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP,
                HybridizationType.SP3,
            ],
        ),
        (
            "OS(=O)(=O)O",  # sulfuric acid
            [
                HybridizationType.SP3,
                HybridizationType.SP3,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP3,
            ],
        ),
        (
            "N(=O)O",
            [
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP3,
            ],
        ),  # nitrous acid
        (
            "O=[Xe](=O)(=O)=O",  # xenon tetroxide
            [
                HybridizationType.SP2,
                HybridizationType.SP3,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
            ],
        ),
        (
            "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
            [
                HybridizationType.SP3,
                HybridizationType.SP3D2,
                HybridizationType.SP3,
                HybridizationType.SP3,
                HybridizationType.SP3,
                HybridizationType.SP3,
                HybridizationType.SP3,
            ],
        ),
        (
            "FBr(F)F",
            [
                HybridizationType.SP3,
                HybridizationType.SP3D,
                HybridizationType.SP3,
                HybridizationType.SP3,
            ],
        ),  # bromine trifluoride
        (
            "[Be](Cl)Cl",
            [
                HybridizationType.SP,
                HybridizationType.SP3,
                HybridizationType.SP3,
            ],
        ),
        (
            "C1=CC=CS1",
            [
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
                HybridizationType.SP2,
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
