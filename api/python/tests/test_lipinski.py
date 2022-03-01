import pytest

from indigo import Indigo
from indigo.lipinski import (
    lipinski_criteria,
    n_aliphatic_cycles,
    n_hydrogen_acceptors,
    n_hydrogen_donors,
    sp3_carbon_ratio,
)

THIOPHENE = "C1=CC=CS1"
NITROGLYCERINUM = "C(C(CO[N+](=O)[O-])O[N+](=O)[O-])O[N+](=O)[O-]"
ERYTHROMYCIN = "CCC1C(C(C(C(=O)C(CC(C(C(C(C(C(=O)O1)C)OC2CC(C(C(O2)C)O)(C)OC)C)OC3C(C(CC(O3)C)N(C)C)O)(C)O)C)C)O)(C)O"
OMEPRAZOLE = "CC1=CN=C(C(=C1OC)C)CS(=O)C2=NC3=C(N2)C=C(C=C3)OC"
CAMPHOR = "CC1(C)C2CCC1(C)C(=O)C2"
MORPHINE = "CN1CCC23C4C1CC5=C2C(=C(C=C5)O)OC3C(C=C4)O"
CORONEN = "c1cc2ccc3ccc4ccc5ccc6ccc1c7c2c3c4c5c67"
LANOSTEROL = "C[C@H](CCC=C(C)C)[C@H]1CC[C@]2(C)C1CCC3=C2CC[C@H]4C(C)(C)[C@@H](O)CC[C@]34C"  # (!!!)


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
        (ERYTHROMYCIN, False),
    ],
)
def test_lipinski_criteria(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert lipinski_criteria(mol) == expecting


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [("N", 0), (THIOPHENE, 0), (NITROGLYCERINUM, 1), (OMEPRAZOLE, 0.294)],
)
def test_sp3_carbon_ratio(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert sp3_carbon_ratio(mol) == expecting


@pytest.mark.parametrize(
    "molecule_smiles, expecting",
    [(THIOPHENE, 0), (NITROGLYCERINUM, 0), (CAMPHOR, 3), (LANOSTEROL, 2)],
)
def test_n_aliphatic_cycles(molecule_smiles, expecting):
    indigo = Indigo()
    mol = indigo.loadMolecule(molecule_smiles)
    assert n_aliphatic_cycles(mol) == expecting
