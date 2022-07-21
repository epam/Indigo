import pytest

from ..helpers import assert_match_query, query_cases


class TestSimilarity:
    @pytest.mark.parametrize("query_id, expected", query_cases("similarity"))
    def test_similarity(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.similarity(molecule, "similarity", "euclid-sub", "0.95, 1")
        assert_match_query(result, expected)
