import time
from pathlib import Path

import pytest
from indigo import Indigo

from bingo_elastic.elastic import ElasticRepository
from bingo_elastic.model.helpers import iterate_file
from bingo_elastic.model.record import IndigoRecord


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
def loaded_sdf(elastic_repository: ElasticRepository) -> IndigoRecord:
    sdf = iterate_file(Path("resources/rand_queries_small.sdf"))
    elastic_repository.index_records(sdf, chunk_size=10)
    time.sleep(5)
    return next(iterate_file(Path("resources/rand_queries_small.sdf")))
