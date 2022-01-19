import pytest

from ..constants import DB_BINGO, DB_POSTGRES
from ..helpers import assert_match_query, query_cases

# db_list = [DB_POSTGRES, DB_BINGO]


@pytest.mark.usefixtures('init_db')
class TestTautomers:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'exact(TAU HYD)'))
    def test_tautomers_exact_tau_hyd(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'tautomers', 'TAU HYD')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'exact(TAU R*)'))
    def test_tautomers_exact_tau_r_asterisk(self, db, entities, query_id,
                                            expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'tautomers', 'TAU R*')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'exact(TAU R-C)'))
    def test_tautomers_exact_tau_r_c(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'tautomers', 'TAU R-C')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'exact(TAU)'))
    def test_tautomers_exact_tau(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'tautomers', 'TAU')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'substructure(TAU HYD)'))
    def test_tautomers_substructure_tau_hyd(self, db, entities, query_id,
                                            expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'tautomers', 'TAU HYD')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'substructure(TAU R*)'))
    def test_tautomers_substructure_tau_r_asterisk(self, db, entities,
                                                   query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'tautomers', 'TAU R*')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'substructure(TAU R-C)'))
    def test_tautomers_substructure_tau_r_c(self, db, entities, query_id,
                                            expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'tautomers', 'TAU R-C')
        assert_match_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('tautomers', 'substructure(TAU)'))
    def test_tautomers_substructure_tau(self, db, entities, query_id,
                                        expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'tautomers', 'TAU')
        assert_match_query(result, expected)
