import unittest

from indigo import Indigo


class TestIndigoBase(unittest.TestCase):
    def setUp(self) -> None:
        self.indigo = Indigo()
