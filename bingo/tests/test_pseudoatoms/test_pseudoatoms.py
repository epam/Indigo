import pytest

from ..helpers import assert_match_query, query_cases


# @pytest.mark.usefixtures('init_db')
class TestPseudoatoms:
    @pytest.mark.parametrize('query_id, expected',
                             query_cases('pseudoatoms', 'exact()'))
    def test_pseudoatoms_exact(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'pseudoatoms')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('pseudoatoms', 'exact(STE)'))
    def test_pseudoatoms_exact_ste(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.exact(molecule, 'pseudoatoms', 'STE')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('pseudoatoms',
                                         'similarity(tanimoto, 0.7, 1)'))
    def test_pseudoatoms_similarity_tanimoto(self, db, entities, query_id,
                                             expected):
        molecule = entities.get(query_id)
        result = db.similarity(molecule, 'pseudoatoms', 'tanimoto, 0.7, 1')
        assert_match_query(result, expected)

    @pytest.mark.parametrize('query_id, expected',
                             query_cases('pseudoatoms', 'substructure()'))
    def test_pseudoatoms_substructure(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.substructure(molecule, 'pseudoatoms')
        assert_match_query(result, expected)
