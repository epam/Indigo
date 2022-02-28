# import pytest
#
# from ..helpers import assert_calculate_query, query_cases
#
#
# class TestInchi:
#     @pytest.mark.parametrize('query_id, expected',
#                              query_cases('inchi', 'inchi()'))
#     def test_inchi(self, db, entities, query_id, expected):
#         molecule = entities.get(query_id)
#         result = db.inchi(molecule)
#         assert_calculate_query(result, expected)
#
#     @pytest.mark.parametrize(
#         'query_id, expected',
#         query_cases('inchi', 'inchi(/DoNotAddH -SUU -SLUUD)')
#     )
#     def test_inchi_do_not_add(self, db, entities, query_id, expected):
#         molecule = entities.get(query_id)
#         result = db.inchi(molecule, '/DoNotAddH -SUU -SLUUD')
#         assert_calculate_query(result, expected)
#
# # InChI doesn't recognise any wrong options.
#
#     # @pytest.mark.parametrize(
#     #     'query_id, expected',
#     #     query_cases('inchi', 'inchi(/VeryWrongOption -AnotherOne)')
#     # )
#     # def test_inchi_wrong_option(self, db, entities, query_id, expected):
#     #     molecule = entities.get(query_id)
#     #     result = db.inchi(molecule, '/VeryWrongOption -AnotherOne')
#     #     assert_calculate_query(result, expected)
#
#     @pytest.mark.parametrize('query_id, expected', query_cases('inchi',
#                                                                'inchikey()'))
#     def test_inchikey(self, db, entities, query_id, expected):
#         molecule = entities.get(query_id)
#         result = db.inchi(molecule, inchikey=True)
#         assert_calculate_query(result, expected)
