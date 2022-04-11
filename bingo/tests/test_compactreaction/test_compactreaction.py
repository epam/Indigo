import pytest

from ..helpers import assert_calculate_query, query_cases


class TestCompactreaction:
    @pytest.mark.parametrize(
        "query_id, expected", query_cases("compactreaction")
    )
    def test_compactreaction(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.compactreaction(reaction)
        assert_calculate_query(result, expected)
