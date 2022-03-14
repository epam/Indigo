from time import sleep

import pytest

from bingo_elastic.elastic import ElasticRepository
from bingo_elastic.model.record import (
    IndigoRecord,
    IndigoRecordMolecule,
    IndigoRecordReaction,
    as_iob,
)
from bingo_elastic.queries import SimilarityMatch


def test_empty_create(indigo_fixture):
    mol = indigo_fixture.createMolecule()
    with pytest.raises(Exception):
        IndigoRecordMolecule(indigo_object=mol)

    def err_handler(instance: IndigoRecord, err_: BaseException) -> None:
        assert isinstance(instance.record_id, str)
        assert isinstance(err_, ValueError)

    IndigoRecordMolecule(indigo_object=mol, error_handler=err_handler)

    IndigoRecordMolecule(indigo_object=mol, skip_errors=True)


def test_create(indigo_fixture):
    mol = indigo_fixture.loadMolecule("N1(CC)C2=C(C(=NC=N2)N)N=C1")
    indigo_record = IndigoRecordMolecule(indigo_object=mol)
    assert len(indigo_record.sim_fingerprint) == 70
    assert len(indigo_record.sub_fingerprint) == 644
    assert len(indigo_record.cmf) == 140


def test_create_without_fingerprint(indigo_fixture):
    mol = indigo_fixture.loadMolecule("[H][H]")
    indigo_record = IndigoRecordMolecule(indigo_object=mol, skip_errors=True)
    assert len(indigo_record.sim_fingerprint) == 0
    assert len(indigo_record.sub_fingerprint) == 0


def test_create_with_name(indigo_fixture, resource_loader):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    indigo_record = IndigoRecordMolecule(indigo_object=mol)
    assert indigo_record.name == "Composition1"


def test_create_reaction(
    elastic_repository_reaction: ElasticRepository,
    indigo_fixture,
    resource_loader,
) -> None:
    reaction = indigo_fixture.loadReactionFromFile(
        resource_loader("reactions/rheadb/58029.rxn")
    )
    indigo_reaction = IndigoRecordReaction(indigo_object=reaction)
    test_smiles = {
        reactant.canonicalSmiles() for reactant in reaction.iterateReactants()
    }
    count_reactants = reaction.countReactants()
    count_products = reaction.countProducts()
    assert isinstance(indigo_reaction, IndigoRecordReaction)
    elastic_repository_reaction.index_record(indigo_reaction)
    sleep(1)
    for found_react in elastic_repository_reaction.filter(
        similarity=SimilarityMatch(indigo_reaction, 0.9)
    ):
        found_react_obj = as_iob(found_react, indigo_fixture)
        assert count_products == found_react_obj.countProducts()
        assert count_reactants == found_react_obj.countReactants()
        for reactant in found_react_obj.iterateReactants():
            assert reactant.canonicalSmiles() in test_smiles
