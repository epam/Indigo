from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        m.aromatize()
        self.assertEqual("c1ccccc1", m.smiles())

    def test_logp_value(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("CU")
        m3 = self.indigo.loadMolecule("CSc1ccc2Sc3ccccc3N(CCC4CCCCN4C)c2c1")
        m4 = self.indigo.loadMolecule("Nc1ccccc1")
        self.assertEqual(m1.logP(), 1.69)
        self.assertEqual(m2.logP(), 0.0)
        self.assertEqual(m3.logP(), 5.89)
        self.assertEqual(m4.logP(), 1.27)

    def test_logp_mr(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("CU")
        m3 = self.indigo.loadMolecule("Clc1ccccc1")
        self.assertEqual(m1.molarRefractivity(), 26.44)
        self.assertEqual(m2.molarRefractivity(), 0.0)
        self.assertEqual(m3.molarRefractivity(), 31.45)

    def test_check_single_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[Na+].C")
        m2 = self.indigo.loadMolecule("[Rb+].C")
        m3 = self.indigo.loadMolecule("[Ca+2].C")
        m4 = self.indigo.loadMolecule("[Zn+2].C")
        m5 = self.indigo.loadMolecule("[Al+3].C")
        m6 = self.indigo.loadMolecule("[Cr+3].C")
        m7 = self.indigo.loadMolecule("[Ru+4].C")
        m8 = self.indigo.loadMolecule("[Sn+4].C")
        m9 = self.indigo.loadMolecule("[Cl-].C")
        m10 = self.indigo.loadMolecule("[F-].C")
        m11 = self.indigo.loadMolecule("[S-2].C")
        m12 = self.indigo.loadMolecule("[Se-2].C")
        self.assertTrue(m1.checkSalt(), "{} contains Na+.".format(m1.smiles()))
        self.assertTrue(m2.checkSalt(), "{} contains Rb+.".format(m2.smiles()))
        self.assertTrue(
            m3.checkSalt(), "{} contains Ca2+.".format(m3.smiles())
        )
        self.assertTrue(
            m4.checkSalt(), "{} contains Zn2+.".format(m4.smiles())
        )
        self.assertTrue(
            m5.checkSalt(), "{} contains Al3+.".format(m5.smiles())
        )
        self.assertTrue(
            m6.checkSalt(), "{} contains Cr3+.".format(m6.smiles())
        )
        self.assertTrue(
            m7.checkSalt(), "{} contains Ru4+.".format(m7.smiles())
        )
        self.assertTrue(
            m8.checkSalt(), "{} contains Sn4+.".format(m8.smiles())
        )
        self.assertTrue(m9.checkSalt(), "{} contains Cl-.".format(m9.smiles()))
        self.assertTrue(
            m10.checkSalt(), "{} contains F-.".format(m10.smiles())
        )
        self.assertTrue(
            m11.checkSalt(), "{} contains S2-.".format(m11.smiles())
        )
        self.assertTrue(
            m12.checkSalt(), "{} contains Se2-.".format(m12.smiles())
        )

    def test_check_simple_molecular_salt(self) -> None:
        m1 = self.indigo.loadMolecule("OS(=O)(=O)O.C")
        m2 = self.indigo.loadMolecule("OP(=O)(O)O.C")
        m3 = self.indigo.loadMolecule("N.C")
        m4 = self.indigo.loadMolecule("OCl(=O)=O.C")
        m5 = self.indigo.loadMolecule("O=[Si]=O.C")
        m6 = self.indigo.loadMolecule("O=[Mn]=O.C")
        m7 = self.indigo.loadMolecule("S=[Fe].C")
        m8 = self.indigo.loadMolecule("Cl[Ag]")
        self.assertTrue(
            m1.checkSalt(), "{} contains sulfuric acid.".format(m1.smiles())
        )
        self.assertTrue(
            m2.checkSalt(), "{} contains phosphoric acid.".format(m2.smiles())
        )
        self.assertTrue(
            m3.checkSalt(), "{} contains ammonia.".format(m3.smiles())
        )
        self.assertTrue(
            m4.checkSalt(), "{} contains chloric acid.".format(m4.smiles())
        )
        self.assertTrue(
            m5.checkSalt(), "{} contains silicon dioxide.".format(m5.smiles())
        )
        self.assertTrue(
            m6.checkSalt(),
            "{} contains manganese dioxide.".format(m6.smiles()),
        )
        self.assertTrue(
            m7.checkSalt(), "{} contains ferrous sulfide.".format(m7.smiles())
        )
        self.assertTrue(
            m8.checkSalt(), "{} contains silver chloride.".format(m8.smiles())
        )

    def test_check_complex_ion(self) -> None:
        m1 = self.indigo.loadMolecule("OP(=O)(O)[O-].C")
        m2 = self.indigo.loadMolecule("OS(=O)(=O)[O-].C")
        m3 = self.indigo.loadMolecule("O[Se](=O)[O-].C")
        m4 = self.indigo.loadMolecule("[N+](=O)([O-])[O-].C")
        m5 = self.indigo.loadMolecule("[O-]N(=O).C")
        m6 = self.indigo.loadMolecule("[O-]I=O.C")
        m7 = self.indigo.loadMolecule("[O-]Cl.C")
        m8 = self.indigo.loadMolecule("[OH-].C")
        self.assertTrue(
            m1.checkSalt(),
            "{} contains dihydrogenphosphate ion.".format(m1.smiles()),
        )
        self.assertTrue(
            m2.checkSalt(),
            "{} contains dihydrogensulfate ion.".format(m2.smiles()),
        )
        self.assertTrue(
            m3.checkSalt(),
            "{} contains hydrogenselenite ion.".format(m3.smiles()),
        )
        self.assertTrue(
            m4.checkSalt(), "{} contains nitrate ion.".format(m4.smiles())
        )
        self.assertTrue(
            m5.checkSalt(), "{} contains nitrite ion.".format(m5.smiles())
        )
        self.assertTrue(
            m6.checkSalt(), "{} contains iodite ion.".format(m6.smiles())
        )
        self.assertTrue(
            m7.checkSalt(), "{} contains hypochlorite ion.".format(m7.smiles())
        )
        self.assertTrue(
            m8.checkSalt(), "{} contains hydroxide ion.".format(m8.smiles())
        )

    def test_check_multiple_ions(self) -> None:
        # TODO: update for counting matches
        m1 = self.indigo.loadMolecule("[Na+].[Cl-].C")
        m2 = self.indigo.loadMolecule("[O-]S(=O)(=O)[O-].[K+].[K+].C")
        self.assertTrue(
            m1.checkSalt(),
            "{} contains sodium and chloride ions.".format(m1.smiles()),
        )
        self.assertTrue(
            m2.checkSalt(),
            "{} contains potassium and sulfate ions.".format(m2.smiles()),
        )

    def test_check_no_ions(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("C1=CC=C2C=CC=CC2=C1")
        self.assertFalse(
            m1.checkSalt(), "{} doesn`t contain any salts.".format(m1.smiles())
        )
        self.assertFalse(
            m2.checkSalt(), "{} doesn`t contain any salts.".format(m2.smiles())
        )

    def test_check_bonded_metal_atom(self) -> None:
        m1 = self.indigo.loadMolecule("CC[Pb](CC)(CC)CC")
        m2 = self.indigo.loadMolecule("C[Al](C)C")
        self.assertFalse(
            m1.checkSalt(), "{} doesn`t contain any salts.".format(m1.smiles())
        )
        self.assertFalse(
            m2.checkSalt(), "{} doesn`t contain any salts.".format(m2.smiles())
        )

    def test_check_bonded_acid_group(self) -> None:
        m1 = self.indigo.loadMolecule("C1=CC=C(C=C1)[N+](=O)[O-]")
        m2 = self.indigo.loadMolecule("C(C(=O)O)S(=O)(=O)O")
        self.assertFalse(
            m1.checkSalt(), "{} doesn`t contain any salts.".format(m1.smiles())
        )
        self.assertFalse(
            m2.checkSalt(), "{} doesn`t contain any salts.".format(m2.smiles())
        )
