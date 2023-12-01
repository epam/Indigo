import unittest

from bingo_elastic.elastic import ElasticRepository, IndexName
from bingo_elastic.model import helpers


class GeneratorTest(unittest.TestCase):
    def test_generator_exception(self):
        sdf = helpers.iterate_sdf("data/big_test.sdf")
        ElasticRepository(IndexName.BINGO_CUSTOM, host="localhost", port=9200).index_records(records=sdf)

if __name__ == '__main__':
    unittest.main()
