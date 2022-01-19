import pytest

from ..constants import DB_POSTGRES
from ..helpers import assert_calculate_query, query_cases

# db_list = [DB_POSTGRES]


@pytest.mark.usefixtures('init_db')
class TestFingerprint:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('fingerprint', 'fingerprint(sim)'))
    def test_fingerprint_sim(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.fingerprint(molecule, 'sim')
        assert_calculate_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('fingerprint', 'fingerprint(sub)'))
    def test_fingerprint_sub(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.fingerprint(molecule, 'sub')
        assert_calculate_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('fingerprint', 'fingerprint(sub-res)'))
    def test_fingerprint_sub_res(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.fingerprint(molecule, 'sub-res')
        assert_calculate_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('fingerprint', 'fingerprint(sub-tau)'))
    def test_fingerprint_sub_tau(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.fingerprint(molecule, 'sub-tau')
        assert_calculate_query(result, expected)

    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('fingerprint', 'fingerprint(full)'))
    def test_fingerprint_full(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.fingerprint(molecule, 'full')
        assert_calculate_query(result, expected)
