import pytest

from ..constants import DB_POSTGRES
from ..helpers import assert_calculate_query, query_cases

# db_list = [DB_POSTGRES]


@pytest.mark.usefixtures('init_db')
class TestRsmiles:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('rsmiles'))
    def test_molecular_weight(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsmiles(reaction)
        assert_calculate_query(result, expected)
