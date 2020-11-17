from pathlib import Path

from bingo_elastic.queries import RangeQuery
from indigo import Indigo

from bingo_elastic.elastic import ElasticRepository, IndigoRecord
from bingo_elastic.model.helpers import iterate_file
from bingo_elastic.predicates import (
    EuclidSimilarityMatch,
    TanimotoSimilarityMatch,
    TverskySimilarityMatch,
    ExactMatch,
)


def test_create_index(elastic_repository: ElasticRepository):
    sdf = iterate_file(Path("resources/rand_queries_small.sdf"))
    elastic_repository.index_records(sdf, chunk_size=10)


def test_similarity_matches(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
):
    for sim_alg in [
        TanimotoSimilarityMatch(loaded_sdf, 0.9),
        EuclidSimilarityMatch(loaded_sdf, 0.9),
        TverskySimilarityMatch(loaded_sdf, 0.9, 0.5, 0.5),
        ExactMatch(loaded_sdf),
    ]:
        result = elastic_repository.filter(similarity=sim_alg)
        assert (
            loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
            == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
        )


def test_filter_by_name(
        elastic_repository: ElasticRepository,
        indigo_fixture: Indigo,
        loaded_sdf: IndigoRecord,
):
    mol = indigo_fixture.loadMoleculeFromFile("resources/composition1.mol")
    elastic_repository.index_record(IndigoRecord(indigo_object=mol))
    result = elastic_repository.filter(name="Composition1")
    for item in result:
        assert item.name == "Composition1"

    result = elastic_repository.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecord(indigo_object=mol), 0.1))

    i = 0
    for _ in result:
        i += 1
    assert i == 20

    result = elastic_repository.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecord(indigo_object=mol), 0.1),
        name="Composition1")

    for item in result:
        assert item.name == "Composition1"


def test_substructure_search(
        elastic_repository: ElasticRepository,
        indigo_fixture: Indigo,
        loaded_sdf: IndigoRecord):
    result = elastic_repository.filter(substructure=loaded_sdf)
    for item in result:
        assert item.as_indigo_object(indigo_fixture).canonicalSmiles() == \
               loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()


def test_range_search(
        elastic_repository: ElasticRepository,
        indigo_fixture: Indigo):
    for i, item in enumerate(iterate_file(Path("resources/rand_queries_small.sdf"))):
        item.ind_number = i
        elastic_repository.index_record(item)
    result = elastic_repository.filter(RangeQuery("ind_number", 1, 10))
    i = 0
    for _ in result:
        i += 1
    assert i == 10
