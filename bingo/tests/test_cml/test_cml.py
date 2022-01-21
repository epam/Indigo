import pytest
import xml.etree.ElementTree as e

from ..constants import DB_POSTGRES
from ..helpers import assert_calculate_query, create_temp_file, query_cases
from ..logger import logger


# db_list = [DB_POSTGRES]


def elements_equal(e1, e2):
    if e1.tag != e2.tag:
        print("FALSE tag")
        return False
    # if e1.text != e2.text:
    #     print("FALSE text")
    #     return False
    if e1.tail != e2.tail:
        print("FALSE tail")
        return False
    if e1.attrib != e2.attrib:
        print("FALSE attrib")
        return False
    if len(e1) != len(e2):
        print("FALSE len")
        return False


@pytest.mark.usefixtures('init_db')
class TestCml:
    # @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('cml'))
    def test_cml(self, db, entities, query_id, expected):
        molecule = entities.get(query_id)
        result = db.cml(molecule)
        # print('RESULT', result)
        # print('EXPECTED', expected)
        # root_res = e.fromstring(result)
        # root_exp = e.fromstring(expected)
        # print('ROOT_RES', root_res)
        # print('ROOT_EXP', root_exp)
        # if result and expected:
        #     assert all(elements_equal(c1, c2) for c1, c2 in zip(root_res, root_exp)) == True
        # assert result == expected
        if result and expected:
            try:
                result = create_temp_file(result)
                expected = create_temp_file(expected)
            except Exception as e:
                logger.error(f"TestCML Error: {str(e)}")

        assert_calculate_query(result, expected)
