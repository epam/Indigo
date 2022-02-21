import pytest

from ..helpers import assert_calculate_query, query_cases


class TestMass:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('mass', 'mass(molecular-weight)'))
    def test_molecular_weight(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.mass(molecule, 'molecular-weight')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('mass', 'mass(most-abundant-mass)'))
    def test_most_abundant_mass(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.mass(molecule, 'most-abundant-mass')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('mass', 'mass(monoisotopic-mass)'))
    def test_monoisotopic_mass(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.mass(molecule, 'monoisotopic-mass')
        assert_calculate_query(result, expected)
