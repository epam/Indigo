import time
from pathlib import Path
from typing import Callable

import pytest
from bingo_elastic.model.record import skip_errors
from indigo import Indigo

from bingo_elastic.elastic import ElasticRepository, IndigoRecord
from bingo_elastic.model.helpers import iterate_file
from bingo_elastic.queries import (EuclidSimilarityMatch, RangeQuery,
                                   TanimotoSimilarityMatch,
                                   TverskySimilarityMatch, WildcardQuery)


def test_create_index(
    elastic_repository: ElasticRepository,
    resource_loader: Callable[[str], str],
):
    sdf = iterate_file(
        Path(resource_loader("resources/rand_queries_small.sdf"))
    )
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
    ]:
        result = elastic_repository.filter(similarity=sim_alg)
        assert (
            loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
            == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
        )


def test_exact_match(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
):
    result = elastic_repository.filter(exact=loaded_sdf)
    assert (
        loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
        == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
    )


def test_filter_by_name(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
    resource_loader: Callable[[str], str],
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("resources/composition1.mol")
    )
    elastic_repository.index_record(IndigoRecord(indigo_object=mol))
    time.sleep(1)
    result = elastic_repository.filter(name="Composition1")
    for item in result:
        assert item.name == "Composition1"

    result = elastic_repository.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecord(indigo_object=mol), 0.1
        )
    )

    i = 0
    for _ in result:
        i += 1
    assert i == 10

    result = elastic_repository.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecord(indigo_object=mol), 0.1
        ),
        name="Composition1",
    )

    for item in result:
        assert item.name == "Composition1"


def test_substructure_search(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
):
    result = elastic_repository.filter(substructure=loaded_sdf)
    for item in result:
        assert (
            item.as_indigo_object(indigo_fixture).canonicalSmiles()
            == loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
        )


def test_range_search(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    resource_loader: Callable[[str], str],
):
    for i, item in enumerate(
        iterate_file(Path(resource_loader("resources/rand_queries_small.sdf")))
    ):
        item.ind_number = i
        elastic_repository.index_record(item)
    result = elastic_repository.filter(ind_number=RangeQuery(1, 10))
    i = 0
    for _ in result:
        i += 1
    assert i == 10


def test_wildcard_search(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
    resource_loader: Callable[[str], str],
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("resources/composition1.mol")
    )
    elastic_repository.index_record(IndigoRecord(indigo_object=mol))
    time.sleep(1)
    result = elastic_repository.filter(name=WildcardQuery("Comp*"))
    for item in result:
        assert item.name == "Composition1"


def test_custom_fields(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecord,
    resource_loader: Callable[[str], str],
):

    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("resources/composition1.mol")
    )
    rec = IndigoRecord(indigo_object=mol,
                       PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N")
    elastic_repository.index_record(rec)
    time.sleep(1)
    result = elastic_repository.filter(
        PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N"
    )
    for item in result:
        assert item.PUBCHEM_IUPAC_INCHIKEY == "RDHQFKQIGNGIED-UHFFFAOYSA-N"


def test_search_empty_fingerprint(
    elastic_repository: ElasticRepository,
    indigo_fixture: Indigo,
    resource_loader: Callable[[str], str],
):
    for smile in ["[H][H]", "[H][F]"]:
        rec = IndigoRecord(
            indigo_object=indigo_fixture.loadMolecule(smile),
            skip_errors=True
        )
        elastic_repository.index_record(rec)
    time.sleep(5)
    result = elastic_repository.filter(
        exact=IndigoRecord(
            indigo_object=indigo_fixture.loadMolecule("[H][H]"),
            skip_errors=True
        )
    )

    assert (
            "[H][H]" == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
    )
    with pytest.raises(StopIteration):
        next(result).as_indigo_object(indigo_fixture).canonicalSmiles()


# # TODO: create pubchem test
# def test_pubchem(indigo_fixture: Indigo,
#                  resource_loader: Callable[[str], str],
#                  elastic_repository: ElasticRepository,):
#
#     iterator = iterate_file(
#         Path(resource_loader("resources/pubchem_1.sdf")),
#         error_handler=skip_errors
#     )
#     elastic_repository.index_records(iterator)
#     pass
