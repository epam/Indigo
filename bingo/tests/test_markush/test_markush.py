import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_match_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]



@pytest.mark.usefixtures('init_db')
class TestMarkush:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('markush'))
    def test_markush(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'markush')
        assert_match_query(result, expected)
