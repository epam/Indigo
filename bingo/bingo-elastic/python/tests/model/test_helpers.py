from pathlib import Path

from bingo_elastic.model import helpers
from bingo_elastic.model.record import (
    IndigoRecordMolecule,
    IndigoRecordReaction,
)


def test_iterate_sdf(resource_loader):
    results = []
    for step in range(0, 2):
        if 0 == step:
            sdf = helpers.iterate_sdf(
                resource_loader("molecules/rand_queries_small.sdf")
            )
        else:
            sdf = helpers.iterate_file(
                Path(resource_loader("molecules/rand_queries_small.sdf"))
            )
        i = 0
        for i, _ in enumerate(sdf, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]


def test_iterate_smiles(resource_loader):
    results = []
    for step in range(0, 2):
        if 0 == step:
            smiles = helpers.iterate_smiles(
                resource_loader("molecules/pubchem_slice_50.smi")
            )
        else:
            smiles = helpers.iterate_file(
                Path(resource_loader("molecules/pubchem_slice_50.smi"))
            )
        i = 0
        for i, _ in enumerate(smiles, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]


def test_iterate_cml(resource_loader):
    results = []
    for step in range(0, 2):
        if 0 == step:
            cml = helpers.iterate_cml(
                resource_loader("molecules/tetrahedral-all.cml")
            )
        else:
            cml = helpers.iterate_file(
                Path(resource_loader("molecules/tetrahedral-all.cml"))
            )
        i = 0
        for i, _ in enumerate(cml, start=1):
            pass
        results.append(i)
    assert results[0] == results[1]


def test_load_reaction(indigo_fixture, resource_loader) -> None:
    reaction = helpers.load_reaction(
        resource_loader("reactions/rheadb/58029.rxn"), indigo_fixture
    )
    assert isinstance(reaction, IndigoRecordReaction)


def test_load_molucule(indigo_fixture, resource_loader) -> None:
    molecule = helpers.load_molecule(
        resource_loader("molecules/composition1.mol"), indigo_fixture
    )
    assert isinstance(molecule, IndigoRecordMolecule)
