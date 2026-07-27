import pytest

from ..helpers import assert_aam_query, query_cases


class TestAam:
    @pytest.mark.parametrize(
        "query_id, expected", query_cases("aam", "aam(ALTER)")
    )
    def test_aam_alter(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, "ALTER")
        assert_aam_query(result, expected, "ALTER")

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("aam", "aam(DISCARD)")
    )
    def test_aam_discard(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, "DISCARD")
        assert_aam_query(result, expected, "DISCARD")

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("aam", "aam(CLEAR)")
    )
    def test_aam_clear(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, "CLEAR")
        assert_aam_query(result, expected, "CLEAR")

    @pytest.mark.parametrize(
        "query_id, expected", query_cases("aam", "aam(KEEP)")
    )
    def test_aam_keep(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.aam(reaction, "KEEP")
        assert_aam_query(result, expected, "KEEP")
