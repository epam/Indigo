import pytest

from ..helpers import assert_calculate_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestGross:
    @pytest.mark.parametrize('query_id, expected', query_cases('gross'))
    def test_gross(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.gross(molecule)
        assert_calculate_query(result, expected)
