import pytest

from ..helpers import assert_match_query, query_cases


class TestMarkush:
    @pytest.mark.parametrize('query_id, expected', query_cases('markush'))
    def test_markush(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'markush')
        assert_match_query(result, expected)
