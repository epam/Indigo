import time
from pathlib import Path
from typing import Callable

import pytest
from indigo import Indigo

from bingo_elastic.elastic import ElasticRepository
from bingo_elastic.model.helpers import iterate_file
from bingo_elastic.model.record import IndigoRecord


@pytest.fixture()
def resource_loader() -> Callable[[str], str]:
    cwd = Path.cwd()

    def wrapper(resource: str):
        if cwd.name == "tests":
            return resource
        if cwd.name == "model":
            return str(cwd.parent / resource)
        return str(cwd / "tests" / resource)

    return wrapper


@pytest.fixture
def indigo_fixture() -> Indigo:
    return Indigo()


@pytest.fixture
def elastic_repository() -> ElasticRepository:
    return ElasticRepository(host="127.0.0.1", port=9200)


@pytest.fixture(autouse=True)
def clear_index(elastic_repository: ElasticRepository):
    elastic_repository.delete_all_records()


@pytest.fixture
def loaded_sdf(
    elastic_repository: ElasticRepository,
    resource_loader: Callable[[str], str],
) -> IndigoRecord:
    resource = resource_loader("resources/rand_queries_small.sdf")
    sdf = iterate_file(Path(resource))
    elastic_repository.index_records(sdf, chunk_size=10)
    time.sleep(5)
    return next(
        iterate_file(Path(resource_loader("resources/rand_queries_small.sdf")))
    )
