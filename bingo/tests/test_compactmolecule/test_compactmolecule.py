import pytest

from ..helpers import assert_calculate_query, query_cases


class TestCompactmolecule:
    @pytest.mark.parametrize(
        "query_id, expected", query_cases("compactmolecule")
    )
    def test_compactmolecule(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.compactmolecule(molecule)
        assert_calculate_query(result, expected)
