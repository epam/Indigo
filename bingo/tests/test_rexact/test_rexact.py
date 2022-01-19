import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_match_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]


@pytest.mark.usefixtures('init_db')
class TestRexact:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rexact', 'rexact()'))
    def test_rexact(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rexact(reaction, 'rexact')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rexact', 'rexact(AAM)'))
    def test_rexact_aam(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rexact(reaction, 'rexact', 'AAM')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rexact', 'rexact(ALL)'))
    def test_rexact_all(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rexact(reaction, 'rexact', 'ALL')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rexact', 'rexact(NONE)'))
    def test_rexact_none(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rexact(reaction, 'rexact', 'NONE')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rexact', 'rexact(STE MAS)'))
    def test_rexact_ste_mas(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rexact(reaction, 'rexact', 'STE MAS')
        assert_match_query(result, expected)
