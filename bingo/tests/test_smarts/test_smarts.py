import pytest

from ..helpers import assert_calculate_query, query_cases, assert_match_query


class TestSmarts:
    @pytest.mark.parametrize('query_id, expected', query_cases('smarts'))
    def test_smarts(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result, info = db.smarts(molecule, 'smarts')
        info = info.replace('%(query_entity)s', "'{}'".format(molecule.rawData()))
        assert_match_query(result, expected, info)
