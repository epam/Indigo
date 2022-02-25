import pytest

from indigo import Indigo
from indigo.lipinski import (
    n_hydrogen_donors,
    n_hydrogen_acceptors,
    lipinski_criteria,
)

THIOPHENE = "C1=CC=CS1"
NITROGLYCERINUM = "C(C(CO[N+](=O)[O-])O[N+](=O)[O-])O[N+](=O)[O-]"
ERYTHROMYCIN = "CCC1C(C(C(C(=O)C(CC(C(C(C(C(C(=O)O1)C)OC2CC(C(C(O2)C)O)(C)OC)C)OC3C(C(CC(O3)C)N(C)C)O)(C)O)C)C)O)(C)O"
OMEPRAZOLE = "CC1=CN=C(C(=C1OC)C)CS(=O)C2=NC3=C(N2)C=C(C=C3)OC"


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (THIOPHENE, 0),
        (NITROGLYCERINUM, 0),
        (OMEPRAZOLE, 1),
        (ERYTHROMYCIN, 5),
    ],
)
def test_n_hydrogen_donors(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert n_hydrogen_donors(mol) == expecting


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (THIOPHENE, 0),
        (NITROGLYCERINUM, 12),
        (OMEPRAZOLE, 6),
        (ERYTHROMYCIN, 14),
    ],
)
def test_n_hydrogen_acceptors(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert n_hydrogen_acceptors(mol) == expecting


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [
        (THIOPHENE, True),
        (NITROGLYCERINUM, True),
        (OMEPRAZOLE, True),
        (ERYTHROMYCIN, False)
    ],
)
def test_lipinski_criteria(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert lipinski_criteria(mol) == expecting
