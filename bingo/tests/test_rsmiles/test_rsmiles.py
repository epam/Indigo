import pytest

from ..helpers import assert_calculate_query, query_cases


class TestRsmiles:
    @pytest.mark.parametrize('query_id, expected', query_cases('rsmiles'))
    def test_molecular_weight(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsmiles(reaction)
        assert_calculate_query(result, expected)
