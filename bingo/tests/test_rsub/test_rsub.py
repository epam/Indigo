import pytest

from ..helpers import assert_match_query, query_cases


class TestRsub:
    @pytest.mark.parametrize('query_id, expected', query_cases('rsub'))
    def test_rsub(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsubstructure(reaction, 'rsub')
        print("RESULT", result)
        print("EXPECTED", expected)
        print("DIFF_RES", set(result) - set(expected))
        print("DIFF_EXP", set(expected) - set(result))
        assert_match_query(result, expected)
