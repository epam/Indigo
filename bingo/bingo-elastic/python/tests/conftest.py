import gc
import time
from pathlib import Path
from typing import Callable

import pytest
from indigo import Indigo  # type: ignore

from bingo_elastic.elastic import (
    AsyncElasticRepository,
    ElasticRepository,
    IndexName,
)
from bingo_elastic.model.helpers import iterate_file, load_reaction
from bingo_elastic.model.record import (
    IndigoRecordMolecule,
    IndigoRecordReaction,
)


@pytest.fixture()
def resource_loader() -> Callable[[str], str]:
    cwd = Path.cwd()

    def wrapper(resource: str):
        if cwd.name == "tests":
            return str(Path("resources") / resource)
        if cwd.name == "model":
            return str(cwd.parent / "resources" / resource)
        return str(cwd / "tests" / "resources" / resource)

    return wrapper


@pytest.fixture
def indigo_fixture() -> Indigo:
    indigo = Indigo()
    yield indigo
    del indigo
    gc.collect()


@pytest.fixture
def elastic_repository_molecule() -> ElasticRepository:
    x = ElasticRepository(
        IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200
    )
    return x


@pytest.fixture
def elastic_repository_reaction() -> ElasticRepository:
    return ElasticRepository(
        IndexName.BINGO_REACTION, host="127.0.0.1", port=9200
    )


@pytest.fixture
def a_elastic_repository_molecule() -> Callable[[], AsyncElasticRepository]:
    def wraped():
        return AsyncElasticRepository(
            IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200
        )

    return wraped


@pytest.fixture
def a_elastic_repository_reaction() -> Callable[[], AsyncElasticRepository]:
    def wraped():
        return AsyncElasticRepository(
            IndexName.BINGO_REACTION, host="127.0.0.1", port=9200
        )

    return wraped


@pytest.fixture(autouse=True)
def clear_index(
    elastic_repository_molecule: ElasticRepository,
    elastic_repository_reaction: ElasticRepository,
):
    elastic_repository_molecule.delete_all_records()
    elastic_repository_reaction.delete_all_records()


@pytest.fixture
def loaded_sdf(
    elastic_repository_molecule: ElasticRepository, resource_loader
) -> IndigoRecordMolecule:
    resource = resource_loader("molecules/rand_queries_small.sdf")
    sdf = iterate_file(Path(resource))
    elastic_repository_molecule.index_records(sdf, chunk_size=10)
    time.sleep(5)
    return next(
        iterate_file(Path(resource_loader("molecules/rand_queries_small.sdf")))
    )


@pytest.fixture
def fixture_molecules_20_10_5_1(
    elastic_repository_molecule: ElasticRepository, indigo_fixture: Indigo
) -> None:
    def generator_records(molecules):
        for x in molecules:
            yield IndigoRecordMolecule(indigo_object=x)

    mol1 = [indigo_fixture.loadMolecule("CCO") for _ in range(20)]
    elastic_repository_molecule.index_records(generator_records(mol1))
    mol2 = [indigo_fixture.loadMolecule("CCCO") for _ in range(10)]
    elastic_repository_molecule.index_records(generator_records(mol2))
    mol3 = [indigo_fixture.loadMolecule("CO") for _ in range(5)]
    elastic_repository_molecule.index_records(generator_records(mol3))
    # We will add one molecule and fake fingerprints and hash to get collisions
    # in order to test postprocess actions
    ccco_mol = IndigoRecordMolecule(
        indigo_object=indigo_fixture.loadMolecule("CCCO")
    )
    mol4 = IndigoRecordMolecule(indigo_object=indigo_fixture.loadMolecule("S"))
    mol4.sub_fingerprint = ccco_mol.sub_fingerprint
    mol4.sim_fingerprint = ccco_mol.sim_fingerprint
    setattr(mol4, "hash", getattr(ccco_mol, "hash"))
    elastic_repository_molecule.index_record(mol4)
    elastic_repository_molecule.el_client.indices.refresh(
        index=IndexName.BINGO_MOLECULE.value
    )


@pytest.fixture
def fixture_reactions_20_10_5_1(
    elastic_repository_reaction: ElasticRepository, indigo_fixture: Indigo
) -> None:
    def generator_records(reactions):
        for x in reactions:
            yield IndigoRecordReaction(indigo_object=x)

    rxn1 = [indigo_fixture.loadReaction("CCO>>CC=O") for _ in range(20)]
    elastic_repository_reaction.index_records(generator_records(rxn1))
    rxn2 = [indigo_fixture.loadReaction("CCCO>>CC=C") for _ in range(10)]
    elastic_repository_reaction.index_records(generator_records(rxn2))
    rxn3 = [indigo_fixture.loadReaction("CO>>CC=O") for _ in range(5)]
    elastic_repository_reaction.index_records(generator_records(rxn3))
    # We will add one reaction and fake fingerprints and hash to get collisions
    # in order to test postprocess actions
    ccco_rxn = IndigoRecordReaction(
        indigo_object=indigo_fixture.loadReaction("CCCO>>CC=C")
    )
    rxn4 = IndigoRecordReaction(
        indigo_object=indigo_fixture.loadReaction("S>>SO")
    )
    rxn4.sub_fingerprint = ccco_rxn.sub_fingerprint
    rxn4.sim_fingerprint = ccco_rxn.sim_fingerprint
    setattr(rxn4, "hash", getattr(ccco_rxn, "hash"))
    elastic_repository_reaction.index_record(rxn4)
    elastic_repository_reaction.el_client.indices.refresh(
        index=IndexName.BINGO_REACTION.value
    )


@pytest.fixture
def loaded_rxns(
    elastic_repository_reaction: ElasticRepository,
    resource_loader: Callable[[str], str],
    indigo_fixture,
):
    for file_ in Path(resource_loader("reactions/rheadb")).iterdir():
        if file_.suffix == ".rxn":
            reaction_file = load_reaction(file_, indigo_fixture)
            elastic_repository_reaction.index_record(reaction_file)

    time.sleep(5)
