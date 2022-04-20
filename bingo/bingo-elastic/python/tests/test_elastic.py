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
from bingo_elastic.model.helpers import iterate_file
from bingo_elastic.model.record import IndigoRecordMolecule, as_iob
from bingo_elastic.queries import (
    EuclidSimilarityMatch,
    RangeQuery,
    TanimotoSimilarityMatch,
    TverskySimilarityMatch,
    WildcardQuery,
)

AsyncRepositoryT = Callable[[], AsyncElasticRepository]


def test_create_index(
    elastic_repository_molecule: ElasticRepository, resource_loader
):
    sdf = iterate_file(
        Path(resource_loader("molecules/rand_queries_small.sdf"))
    )
    elastic_repository_molecule.index_records(sdf, chunk_size=10)


@pytest.mark.asyncio
async def test_a_create_index(
    a_elastic_repository_molecule: AsyncRepositoryT, resource_loader
):
    sdf = iterate_file(
        Path(resource_loader("molecules/rand_queries_small.sdf"))
    )
    async with a_elastic_repository_molecule() as rep:
        await rep.index_records(sdf, chunk_size=10)


@pytest.mark.asyncio
async def test_a_cm_create_index(resource_loader):
    sdf = iterate_file(
        Path(resource_loader("molecules/rand_queries_small.sdf"))
    )
    async with AsyncElasticRepository(
        IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200
    ) as elastic_rep:
        await elastic_rep.index_records(sdf, chunk_size=10)


def test_similarity_matches(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    for sim_alg in [
        TanimotoSimilarityMatch(loaded_sdf, 0.9),
        EuclidSimilarityMatch(loaded_sdf, 0.9),
        TverskySimilarityMatch(loaded_sdf, 0.9, 0.5, 0.5),
    ]:
        result = elastic_repository_molecule.filter(similarity=sim_alg)
        assert (
            loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
            == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
        )


@pytest.mark.asyncio
async def test_a_similarity_matches(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    for sim_alg in [
        TanimotoSimilarityMatch(loaded_sdf, 0.9),
        EuclidSimilarityMatch(loaded_sdf, 0.9),
        TverskySimilarityMatch(loaded_sdf, 0.9, 0.5, 0.5),
    ]:
        async with a_elastic_repository_molecule() as rep:
            result = rep.filter(similarity=sim_alg)
            async for mol in result:
                assert (
                    loaded_sdf.as_indigo_object(
                        indigo_fixture
                    ).canonicalSmiles()
                    == mol.as_indigo_object(indigo_fixture).canonicalSmiles()
                )
                break


def test_exact_match(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    result = elastic_repository_molecule.filter(exact=loaded_sdf)
    assert (
        loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
        == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
    )


@pytest.mark.asyncio
async def test_a_exact_match(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(exact=loaded_sdf)
        async for mol in result:
            assert (
                loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
                == mol.as_indigo_object(indigo_fixture).canonicalSmiles()
            )
            break


@pytest.mark.asyncio
async def test_filter_by_name(
    elastic_repository_molecule: ElasticRepository,
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,  # pylint: disable=unused-argument
    resource_loader,
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    elastic_repository_molecule.index_record(
        IndigoRecordMolecule(indigo_object=mol)
    )
    time.sleep(1)

    ##################################################################

    # Sync test
    result = elastic_repository_molecule.filter(name="Composition1")
    for item in result:
        assert item.name == "Composition1"

    # Async test
    async with a_elastic_repository_molecule() as rep:
        async for item in rep.filter(name="Composition1"):
            assert item.name == "Composition1"

    ##################################################################

    # Sync test
    result = elastic_repository_molecule.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecordMolecule(indigo_object=mol), 0.1
        )
    )

    i = 0
    for _ in result:
        i += 1
    assert i == 10

    # Async test
    async with a_elastic_repository_molecule() as rep:
        i = 0
        async for _ in rep.filter(
            similarity=TanimotoSimilarityMatch(
                IndigoRecordMolecule(indigo_object=mol), 0.1
            )
        ):
            i += 1
        assert i == 10

    ##################################################################

    # Sync test

    result = elastic_repository_molecule.filter(
        similarity=TanimotoSimilarityMatch(
            IndigoRecordMolecule(indigo_object=mol), 0.1
        ),
        name="Composition1",
    )

    for item in result:
        assert item.name == "Composition1"

    # Async test
    async with a_elastic_repository_molecule() as rep:
        async for item in rep.filter(
            similarity=TanimotoSimilarityMatch(
                IndigoRecordMolecule(indigo_object=mol), 0.1
            ),
            name="Composition1",
        ):
            assert item.name == "Composition1"


def test_substructure_search(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    result = elastic_repository_molecule.filter(substructure=loaded_sdf)
    for item in result:
        assert (
            item.as_indigo_object(indigo_fixture).canonicalSmiles()
            == loaded_sdf.as_indigo_object(indigo_fixture).canonicalSmiles()
        )


@pytest.mark.asyncio
async def test_a_substructure_search(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
):
    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(substructure=loaded_sdf)
        async for item in result:
            assert (
                item.as_indigo_object(indigo_fixture).canonicalSmiles()
                == loaded_sdf.as_indigo_object(
                    indigo_fixture
                ).canonicalSmiles()
            )


def test_range_search(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    resource_loader,
):
    for i, item in enumerate(
        iterate_file(Path(resource_loader("molecules/rand_queries_small.sdf")))
    ):
        item.ind_number = i  # type: ignore
        elastic_repository_molecule.index_record(item)
    result = elastic_repository_molecule.filter(ind_number=RangeQuery(1, 10))
    i = 0
    for _ in result:
        i += 1
    assert i == 10


@pytest.mark.asyncio
async def test_a_range_search(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    resource_loader,
):
    async with a_elastic_repository_molecule() as rep:
        for i, item in enumerate(
            iterate_file(
                Path(resource_loader("molecules/rand_queries_small.sdf"))
            )
        ):
            item.ind_number = i  # type: ignore
            await rep.index_record(item)

    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(ind_number=RangeQuery(1, 10))
        i = 0
        async for _ in result:
            i += 1
        assert i == 10


def test_wildcard_search(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
    resource_loader,
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    elastic_repository_molecule.index_record(
        IndigoRecordMolecule(indigo_object=mol)
    )
    time.sleep(1)
    result = elastic_repository_molecule.filter(name=WildcardQuery("Comp*"))
    for item in result:
        assert item.name == "Composition1"


@pytest.mark.asyncio
async def test_a_wildcard_search(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
    resource_loader,
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    async with a_elastic_repository_molecule() as rep:
        rep.index_record(IndigoRecordMolecule(indigo_object=mol))
    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(name=WildcardQuery("Comp*"))
        async for item in result:
            assert item.name == "Composition1"


def test_custom_fields(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
    resource_loader,
):

    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    rec = IndigoRecordMolecule(
        indigo_object=mol, PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N"
    )
    elastic_repository_molecule.index_record(rec)
    time.sleep(1)
    result = elastic_repository_molecule.filter(
        PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N"
    )
    for item in result:
        iupac_inch = item.PUBCHEM_IUPAC_INCHIKEY  # type: ignore
        assert iupac_inch == "RDHQFKQIGNGIED-UHFFFAOYSA-N"


@pytest.mark.asyncio
async def test_a_custom_fields(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    loaded_sdf: IndigoRecordMolecule,
    resource_loader,
):
    mol = indigo_fixture.loadMoleculeFromFile(
        resource_loader("molecules/composition1.mol")
    )
    rec = IndigoRecordMolecule(
        indigo_object=mol, PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N"
    )
    async with a_elastic_repository_molecule() as rep:
        rep.index_record(rec)

    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(
            PUBCHEM_IUPAC_INCHIKEY="RDHQFKQIGNGIED-UHFFFAOYSA-N"
        )
        async for item in result:
            iupac_inch = item.PUBCHEM_IUPAC_INCHIKEY  # type: ignore
            assert iupac_inch == "RDHQFKQIGNGIED-UHFFFAOYSA-N"


def test_search_empty_fingerprint(
    elastic_repository_molecule: ElasticRepository,
    indigo_fixture: Indigo,
    resource_loader,
):
    for smile in ["[H][H]", "[H][F]"]:
        rec = IndigoRecordMolecule(
            indigo_object=indigo_fixture.loadMolecule(smile), skip_errors=True
        )
        elastic_repository_molecule.index_record(rec)
    time.sleep(5)
    result = elastic_repository_molecule.filter(
        exact=IndigoRecordMolecule(
            indigo_object=indigo_fixture.loadMolecule("[H][H]"),
            skip_errors=True,
        )
    )

    assert (
        "[H][H]"
        == next(result).as_indigo_object(indigo_fixture).canonicalSmiles()
    )
    with pytest.raises(StopIteration):
        next(result).as_indigo_object(indigo_fixture).canonicalSmiles()


@pytest.mark.asyncio
async def test_a_search_empty_fingerprint(
    a_elastic_repository_molecule: AsyncRepositoryT,
    indigo_fixture: Indigo,
    resource_loader,
):
    async with a_elastic_repository_molecule() as rep:
        for smile in ["[H][H]", "[H][F]"]:
            rec = IndigoRecordMolecule(
                indigo_object=indigo_fixture.loadMolecule(smile),
                skip_errors=True,
            )
            await rep.index_record(rec)

    async with a_elastic_repository_molecule() as rep:
        result = rep.filter(
            exact=IndigoRecordMolecule(
                indigo_object=indigo_fixture.loadMolecule("[H][H]"),
                skip_errors=True,
            )
        )

        async for mol in result:
            assert (
                "[H][H]"
                == mol.as_indigo_object(indigo_fixture).canonicalSmiles()
            )


def test_similaririty_matches_reactions(
    elastic_repository_reaction: ElasticRepository,
    loaded_rxns,
    resource_loader,
    indigo_fixture,
) -> None:

    reaction = indigo_fixture.loadReactionFromFile(
        resource_loader("reactions/rheadb/50353.rxn")
    )

    reaction_rec = IndigoRecordMolecule(indigo_object=reaction)

    for found_reaction in elastic_repository_reaction.filter(
        similarity=TanimotoSimilarityMatch(reaction_rec, 0.99)
    ):
        assert (
            as_iob(found_reaction, indigo_fixture).countReactants()
            == reaction.countReactants()
        )

    for found_reaction in elastic_repository_reaction.filter(
        similarity=EuclidSimilarityMatch(reaction_rec, 0.99)
    ):
        assert (
            as_iob(found_reaction, indigo_fixture).countReactants()
            == reaction.countReactants()
        )

    for found_reaction in elastic_repository_reaction.filter(
        similarity=TverskySimilarityMatch(reaction_rec, 0.99)
    ):
        assert (
            as_iob(found_reaction, indigo_fixture).countReactants()
            == reaction.countReactants()
        )

    for found_reaction in elastic_repository_reaction.filter(
        exact=reaction_rec
    ):
        assert (
            as_iob(found_reaction, indigo_fixture).countReactants()
            == reaction.countReactants()
        )


@pytest.mark.asyncio
async def test_a_similaririty_matches_reactions(
    a_elastic_repository_reaction: AsyncRepositoryT,
    loaded_rxns,
    resource_loader,
    indigo_fixture,
) -> None:

    reaction = indigo_fixture.loadReactionFromFile(
        resource_loader("reactions/rheadb/50353.rxn")
    )

    reaction_rec = IndigoRecordMolecule(indigo_object=reaction)

    async with a_elastic_repository_reaction() as rep:
        async for found_reaction in rep.filter(
            similarity=TanimotoSimilarityMatch(reaction_rec, 0.99)
        ):
            assert (
                as_iob(found_reaction, indigo_fixture).countReactants()
                == reaction.countReactants()
            )

        async for found_reaction in rep.filter(
            similarity=EuclidSimilarityMatch(reaction_rec, 0.99)
        ):
            assert (
                as_iob(found_reaction, indigo_fixture).countReactants()
                == reaction.countReactants()
            )

        async for found_reaction in rep.filter(
            similarity=TverskySimilarityMatch(reaction_rec, 0.99)
        ):
            assert (
                as_iob(found_reaction, indigo_fixture).countReactants()
                == reaction.countReactants()
            )

        async for found_reaction in rep.filter(exact=reaction_rec):
            assert (
                as_iob(found_reaction, indigo_fixture).countReactants()
                == reaction.countReactants()
            )
