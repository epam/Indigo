import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestSgroups:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('sgroups', 'exact()'))
    def test_sgroups_exact(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'sgroups')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('sgroups', 'substructure()'))
    def test_sgroups_substructure(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'sgroups')
        assert_match_query(result, expected)
