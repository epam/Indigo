# import pytest
#
# from ..helpers import assert_calculate_query, query_cases
#
#
# class TestSmarts:
#     @pytest.mark.parametrize('query_id, expected', query_cases('smarts'))
#     def test_smarts(self, db, entities, query_id, expected):
#         molecule = entities.get(query_id)
#         result = db.smarts(molecule, 'smarts')
#         print("RESULT", result)
#         print("EXPECTED", expected)
#         assert_calculate_query(result, expected)
