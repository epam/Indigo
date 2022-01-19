import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_match_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]


@pytest.mark.usefixtures('init_db')
class TestRsub:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('rsub'))
    def test_rsub(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsubstructure(reaction, 'rsub')
        assert_match_query(result, expected)
