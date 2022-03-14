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
from bingo_elastic.model.record import IndigoRecordMolecule


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
    return Indigo()


@pytest.fixture
def elastic_repository_molecule() -> ElasticRepository:
    return ElasticRepository(
        IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200
    )


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
