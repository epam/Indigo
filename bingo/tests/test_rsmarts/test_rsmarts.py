# import pytest
#
# from ..helpers import assert_calculate_query, query_cases
#
#
# class TestRsmarts:
#     @pytest.mark.parametrize('query_id, expected', query_cases('rsmarts'))
#     def test_rsmarts(self, db, entities, query_id, expected):
#         reaction = entities.get(query_id)
#         result = db.rsmarts(reaction, 'rsmarts')
#         assert_calculate_query(result, expected)
