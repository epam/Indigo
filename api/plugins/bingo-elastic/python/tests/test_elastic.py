from pathlib import Path

from bingo_elastic.elastic import ElasticRepository
from bingo_elastic.model.helpers import iterate_file


def test_create_index(elastic_repository: ElasticRepository):
    sdf = iterate_file(Path("resources/pubchem_slice_50.smi"))
    elastic_repository.index_records(sdf, chunk_size=10)
