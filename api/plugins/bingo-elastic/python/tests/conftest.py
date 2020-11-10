import pytest
from bingo_elastic.elastic import ElasticRepository
from indigo import Indigo


@pytest.fixture
def indigo_fixture() -> Indigo:
    return Indigo()


@pytest.fixture
def elastic_repository() -> ElasticRepository:
    return ElasticRepository(host="127.0.0.1", port=9200)


@pytest.fixture(autouse=True)
def clear_index(elastic_repository: ElasticRepository):
    elastic_repository.delete_all_records()
