# import pytest
#
# from ..helpers import assert_calculate_query, query_cases
#
#
# class TestRcml:
#     @pytest.mark.parametrize('query_id, expected', query_cases('rcml'))
#     def test_rcml(self, db, entities, query_id, expected):
#         reaction = entities.get(query_id)
#         result = db.rcml(reaction)
#         if result and expected:
#             assert_calculate_query(result, expected)
