import pytest

from ..helpers import assert_calculate_query, query_cases


class TestCheckreaction:
    @pytest.mark.parametrize('query_id, expected', query_cases('checkreaction'))
    def test_checkreaction(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.checkreaction(reaction)
        assert_calculate_query(result, expected)
