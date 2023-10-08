import pytest

from ..helpers import assert_calculate_query, query_cases
from ..logger import logger

class TestRsmiles:
    @pytest.mark.parametrize("query_id, expected", query_cases("rsmiles"))
    def test_molecular_weight(self, db, entities, query_id, expected):
        reaction = entities.get(query_id)
        result = db.rsmiles(reaction)
        if result != expected:
            logger.info(query_id)
            logger.info(result)
            logger.info(expected)
        assert_calculate_query(result, expected)
