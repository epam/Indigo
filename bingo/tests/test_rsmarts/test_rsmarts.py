import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_calculate_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]


@pytest.mark.usefixtures('init_db')
class TestRsmarts:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('rsmarts'))
    def test_rsmarts(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsmarts(reaction, 'rsmarts')
        assert_calculate_query(result, expected)
