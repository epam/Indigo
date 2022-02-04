import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestSubstructure:
    @pytest.mark.parametrize('query_id, expected', query_cases('substructure'))
    def test_substructure(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'substructure')
        assert_match_query(result, expected)
