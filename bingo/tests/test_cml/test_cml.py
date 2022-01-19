import pytest

from ..constants import DB_POSTGRES
from ..helpers import assert_calculate_query, create_temp_file, query_cases
from ..logger import logger

# db_list = [DB_POSTGRES]


@pytest.mark.usefixtures('init_db')
class TestCml:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('cml'))
    def test_cml(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.cml(molecule)
        if result and expected:
            try:
                result = create_temp_file(result)
                expected = create_temp_file(expected)
            except Exception as e:
                logger.error(f"TestCML Error: {str(e)}")

        assert_calculate_query(result, expected)
