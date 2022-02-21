import pytest

from ..helpers import assert_calculate_query, query_cases


class TestRfingerprint:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rfingerprint', 'rfingerprint(full)'))
    def test_rfingerprint_full(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rfingerprint(reaction, 'full')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rfingerprint', 'rfingerprint(sim)'))
    def test_rfingerprint_sim(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rfingerprint(reaction, 'sim')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rfingerprint', 'rfingerprint(sub)'))
    def test_rfingerprint_sub(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rfingerprint(reaction, 'sub')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rfingerprint',
                                         'rfingerprint(sub-res)'))
    def test_rfingerprint_sub_res(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rfingerprint(reaction, 'sub-res')
        assert_calculate_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('rfingerprint', 'rfingerprint('
                                                         'sub-tau)'))
    def test_rfingerprint_sub_tau(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rfingerprint(reaction, 'sub-tau')
        assert_calculate_query(result, expected)
