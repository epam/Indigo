import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestBigtable:
    @pytest.mark.parametrize('query_id, expected', query_cases('bigtable'))
    def test_bigtable(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'bigtable')
        assert_match_query(result, expected)
