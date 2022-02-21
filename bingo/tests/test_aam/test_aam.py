import pytest

from ..helpers import assert_calculate_query, query_cases


class TestAam:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('aam', 'aam(ALTER)'))
    def test_aam_alter(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, 'ALTER')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('aam', 'aam(DISCARD)'))
    def test_aam_discard(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, 'DISCARD')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('aam', 'aam(CLEAR)'))
    def test_aam_clear(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, 'CLEAR')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('aam', 'aam(KEEP)'))
    def test_aam_keep(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, 'KEEP')
        assert_calculate_query(result, expected)
