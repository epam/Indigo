from tests import TestIndigoBase

from indigo.inchi import IndigoInchi


class TestIndigoInchi(TestIndigoBase):
    def setUp(self) -> None:
        super().setUp()
        self.indigo_inchi = IndigoInchi(self.indigo)

    def test_benzene_inchi(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        inchi = self.indigo_inchi.getInchi(m)
        self.assertTrue(inchi)
        self.assertEqual("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi)
