import pytest

from ..constants import DB_POSTGRES
from ..helpers import assert_calculate_query, query_cases

db_list = [DB_POSTGRES]


@pytest.mark.usefixtures('init_db')
class TestGross:
    @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('gross'))
    def test_gross(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.gross(molecule)
        assert_calculate_query(result, expected)
