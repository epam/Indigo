from python.tests import TestIndigoBase


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

    def test_mr_value(self) -> None:
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
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Na+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Rb+.")
        self.assertTrue(m3.checkSalt(), f"{m3.smiles()} contains Ca2+.")
        self.assertTrue(m4.checkSalt(), f"{m4.smiles()} contains Zn2+.")
        self.assertTrue(m5.checkSalt(), f"{m5.smiles()} contains Al3+.")
        self.assertTrue(m6.checkSalt(), f"{m6.smiles()} contains Cr3+.")
        self.assertTrue(m7.checkSalt(), f"{m7.smiles()} contains Ru4+.")
        self.assertTrue(m8.checkSalt(), f"{m8.smiles()} contains Sn4+.")
        self.assertTrue(m9.checkSalt(), f"{m9.smiles()} contains Cl-.")
        self.assertTrue(m10.checkSalt(), f"{m10.smiles()} contains F-.")
        self.assertTrue(m11.checkSalt(), f"{m11.smiles()} contains S2-.")
        self.assertTrue(m12.checkSalt(), f"{m12.smiles()} contains Se2-.")

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
            m1.checkSalt(), f"{m1.smiles()} contains sulfuric acid."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains phosphoric acid."
        )
        self.assertTrue(m3.checkSalt(), f"{m3.smiles()} contains ammonia.")
        self.assertTrue(
            m4.checkSalt(), f"{m4.smiles()} contains chloric acid."
        )
        self.assertTrue(
            m5.checkSalt(), f"{m5.smiles()} contains silicon dioxide."
        )
        self.assertTrue(
            m6.checkSalt(), f"{m6.smiles()} contains manganese dioxide."
        )
        self.assertTrue(
            m7.checkSalt(), f"{m7.smiles()} contains ferrous sulfide."
        )
        self.assertTrue(
            m8.checkSalt(), f"{m8.smiles()} contains silver chloride."
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
            m1.checkSalt(), f"{m1.smiles()} contains dihydrogenphosphate ion."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains dihydrogensulfate ion."
        )
        self.assertTrue(
            m3.checkSalt(), f"{m3.smiles()} contains hydrogenselenite ion."
        )
        self.assertTrue(m4.checkSalt(), f"{m4.smiles()} contains nitrate ion.")
        self.assertTrue(m5.checkSalt(), f"{m5.smiles()} contains nitrite ion.")
        self.assertTrue(m6.checkSalt(), f"{m6.smiles()} contains iodite ion.")
        self.assertTrue(
            m7.checkSalt(), f"{m7.smiles()} contains hypochlorite ion."
        )
        self.assertTrue(
            m8.checkSalt(), f"{m8.smiles()} contains hydroxide ion."
        )

    def test_check_multiple_ions(self) -> None:
        # TODO: update for counting matches
        m1 = self.indigo.loadMolecule("[Na+].[Cl-].C")
        m2 = self.indigo.loadMolecule("[O-]S(=O)(=O)[O-].[K+].[K+].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains sodium and chloride ions."
        )
        self.assertTrue(
            m2.checkSalt(),
            f"{m2.smiles()} contains potassium and sulfate ions.",
        )

    def test_check_no_ions(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("C1=CC=C2C=CC=CC2=C1")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_bonded_metal_atom(self) -> None:
        m1 = self.indigo.loadMolecule("CC[Pb](CC)(CC)CC")
        m2 = self.indigo.loadMolecule("C[Al](C)C")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_bonded_acid_group(self) -> None:
        m1 = self.indigo.loadMolecule("C1=CC=C(C=C1)[N+](=O)[O-]")
        m2 = self.indigo.loadMolecule("C(C(=O)O)S(=O)(=O)O")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_salt_stripping(self) -> None:
        # No disconnected components
        m1 = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1")

        # Single counterion
        m2 = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")

        # Many inorganic ions and water molecules
        m3 = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.O.O.O.O.O.O.O.O.O.O.[Cl-].[Cl-]"
        )

        # Two organic molecules with single counterion
        m4 = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "[O-]S(=O)(=O)[O-]"
        )

        # Complex organic salt
        m5 = self.indigo.loadMolecule(
            "C(C(C(C(C(C(=O)O)O)O)O)O)O.C(C(C(C(C(C(=O)O)O)O)O)O)O.[Fe]"
        )

        # No organic components
        m6 = self.indigo.loadMolecule("[NH4+].[O-]P(=O)([O-])[O-].[Fe+2]")

        self.assertEqual(
            m1.stripSalt().smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1"
        )
        self.assertEqual(
            m2.stripSalt().smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1"
        )
        self.assertEqual(
            m3.stripSalt().smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1"
        )
        self.assertEqual(
            m4.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
        )
        self.assertEqual(
            m5.stripSalt().smiles(),
            "C(O)C(O)C(O)C(O)C(O)C(O)=O.C(O)C(O)C(O)C(O)C(O)C(O)=O.[Fe]",
        )
        self.assertEqual(m6.stripSalt().smiles(), "")

    def test_salt_stripping_options(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
        m_strip = m.stripSalt()
        m.stripSalt(inplace=True)

        self.assertEqual(m_strip.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertNotEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.[Cl-]")
