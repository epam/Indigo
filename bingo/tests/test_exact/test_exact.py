from os import O_NDELAY

import pytest

from ..helpers import assert_match_query, query_cases


class TestExact:
    @pytest.mark.parametrize(
        "query_id, expected", query_cases("exact", "exact()")
    )
    def test_exact(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, "exact")
        assert_match_query(result, expected)

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("exact", "exact(NONE)")
    )
    def test_exact_none(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, "exact", "NONE")
        assert_match_query(result, expected)

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("exact", "exact(ELE STE)")
    )
    def test_exact_ele_ste(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, "exact", "ELE STE")
        assert_match_query(result, expected)

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("exact", "exact(MAS FRA 0.1)")
    )
    def test_exact_mas_fra_01(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, "exact", "MAS FRA 0.1")
        assert_match_query(result, expected)

    def test_exact_join_postgres(self, db, db_backend):
        if db_backend != "postgres":
            pytest.skip("JOIN tests only supported in PostgreSQL backend")
        with open(
            "data/molecules/exact/import/targets/exact_join.sql"
        ) as file:
            result = db._connect.execute(file.read())
        assert result is not None
        out = [x for x in result]
        assert len(out) == 1
        assert out[0][0] == 736832
