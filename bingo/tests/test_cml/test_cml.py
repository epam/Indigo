import pytest

from ..helpers import assert_calculate_query, query_cases


class TestCml:
    @pytest.mark.parametrize("query_id, expected", query_cases("cml"))
    def test_cml(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.cml(molecule)
        if result and expected:
            assert_calculate_query(result, expected)
