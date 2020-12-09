import unittest

from indigo import Indigo

from indigo_renderer import IndigoRenderer


class TestRenderer(unittest.TestCase):
    def test_init(self):
        indigo = Indigo()
        indigo_render = IndigoRenderer(indigo)
        self.assertIsNotNone(indigo_render)


if __name__ == "__main__":
    unittest.main()
