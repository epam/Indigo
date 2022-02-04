import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestResonance:
    @pytest.mark.parametrize('query_id, expected', query_cases('resonance'))
    def test_resonance(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'resonance', 'RES')
        assert_match_query(result, expected)
