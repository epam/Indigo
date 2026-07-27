from indigo.inchi import IndigoInchi
from tests import TestIndigoBase


class TestIndigoInchi(TestIndigoBase):
    def setUp(self) -> None:
        super().setUp()
        self.indigo_inchi = IndigoInchi(self.indigo)

    def test_benzene_inchi(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        expected = "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H"

        inchi = self.indigo_inchi.getInchi(m)
        self.assertTrue(inchi)
        self.assertEqual(expected, inchi)

        inchi = self.indigo_inchi.getInchi(m, None)
        self.assertTrue(inchi)
        self.assertEqual(expected, inchi)

        inchi = self.indigo_inchi.getInchi(m, "")
        self.assertTrue(inchi)
        self.assertEqual(expected, inchi)

        inchi = self.indigo_inchi.getInchi(m, "/WarnOnEmptyStructure")
        self.assertTrue(inchi)
        self.assertEqual(expected, inchi)

        inchi = self.indigo_inchi.getInchi(m, "-WarnOnEmptyStructure")
        self.assertTrue(inchi)
        self.assertEqual(expected, inchi)

    def test_or_enantiomer(self) -> None:
        or_enantiomer = (
            "\n"
            "  TestStruct 2\n"
            "\n"
            "  0  0  0     0  0            999 V3000\n"
            "M  V30 BEGIN CTAB\n"
            "M  V30 COUNTS 8 8 0 0 1\n"
            "M  V30 BEGIN ATOM\n"
            "M  V30 1 C 9.5653 -6.4929 0 0\n"
            "M  V30 2 C 9.5653 -7.3196 0 0\n"
            "M  V30 3 C 10.2813 -7.733 0 0\n"
            "M  V30 4 C 10.9972 -7.3196 0 0 CFG=2\n"
            "M  V30 5 C 10.9972 -6.4929 0 0 CFG=1\n"
            "M  V30 6 C 10.2813 -6.0795 0 0\n"
            "M  V30 7 O 11.7131 -6.0795 0 0\n"
            "M  V30 8 N 11.7131 -7.7329 0 0\n"
            "M  V30 END ATOM\n"
            "M  V30 BEGIN BOND\n"
            "M  V30 1 1 1 6\n"
            "M  V30 2 1 1 2\n"
            "M  V30 3 1 2 3\n"
            "M  V30 4 1 3 4\n"
            "M  V30 5 1 4 5\n"
            "M  V30 6 1 5 6\n"
            "M  V30 7 1 5 7 CFG=1\n"
            "M  V30 8 1 4 8 CFG=1\n"
            "M  V30 END BOND\n"
            "M  V30 BEGIN COLLECTION\n"
            "M  V30 MDLV30/STEREL1 ATOMS=(2 5 4)\n"
            "M  V30 END COLLECTION\n"
            "M  V30 END CTAB\n"
            "M  END\n"
        )
        m = self.indigo.loadMolecule(or_enantiomer)
        inchi = self.indigo_inchi.getInchi(m, "/SRel")
        self.assertTrue(inchi)
        self.assertEqual(
            "InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2",
            inchi,
        )
