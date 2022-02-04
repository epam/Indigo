import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestExact:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('exact', 'exact()'))
    def test_exact(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'exact')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('exact', 'exact(NONE)'))
    def test_exact_none(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'exact', 'NONE')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('exact', 'exact(ELE STE)'))
    def test_exact_ele_ste(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'exact', 'ELE STE')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('exact', 'exact(MAS FRA 0.1)'))
    def test_exact_mas_fra_01(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'exact', 'MAS FRA 0.1')
        assert_match_query(result, expected)
