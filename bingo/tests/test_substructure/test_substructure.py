import pytest

from ..constants import DB_BINGO, DB_BINGO_ELASTIC, DB_POSTGRES
from ..helpers import assert_match_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO, DB_BINGO_ELASTIC]


@pytest.mark.usefixtures('init_db')
class TestSubstructure:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('substructure'))
    def test_substructure(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'substructure')
        assert_match_query(result, expected)
