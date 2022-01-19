import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_calculate_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]


@pytest.mark.usefixtures('init_db')
class TestSmarts:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('smarts'))
    def test_smarts(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.smarts(molecule, 'smarts')
        assert_calculate_query(result, expected)
