from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        m.aromatize()
        self.assertEqual("c1ccccc1", m.smiles())

    def test_check_salt_monovalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Na+].C")
        m2 = self.indigo.loadMolecule("[Rb+].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Na+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Rb+.")

    def test_check_salt_divalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Ca+2].C")
        m2 = self.indigo.loadMolecule("[Zn+2].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Ca2+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Zn2+.")

    def test_check_salt_trivalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Al+3].C")
        m2 = self.indigo.loadMolecule("[Cr+3].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Al3+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Cr3+.")

    def test_check_salt_tetravalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Ru+4].C")
        m2 = self.indigo.loadMolecule("[Sn+4].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Ru4+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Sn4+.")

    def test_check_salt_monovalent_monoatomic_anion(self) -> None:
        m1 = self.indigo.loadMolecule("[Cl-].C")
        m2 = self.indigo.loadMolecule("[F-].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Cl-.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains F-.")

    def test_check_salt_divalent_monoatomic_anion(self) -> None:
        m1 = self.indigo.loadMolecule("[S-2].C")
        m2 = self.indigo.loadMolecule("[Se-2].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains S2-.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Se2-.")

    def test_check_salt_molecular_primary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("S=[Fe].C")
        m2 = self.indigo.loadMolecule("Cl[Ag]")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains ferrous sulfide."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains silver chloride."
        )

    def test_check_salt_molecular_secondary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("S=[Sn]=S.C")
        m2 = self.indigo.loadMolecule("O=[Mn]=O.C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains tin sulfide.")
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains manganese dioxide."
        )

    def test_check_salt_molecular_tertiary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("Cl[Fe](Cl)Cl.C")
        m2 = self.indigo.loadMolecule("OCl(=O)=O.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains ferric chloride."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains chloric acid."
        )

    def test_check_salt_molecular_quaternary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("OS(=O)(=O)O.C")
        m2 = self.indigo.loadMolecule("OP(=O)(O)O.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains sulfuric acid."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains phosphoric acid."
        )

    def test_check_salt_complex_primary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[OH-].C")
        m2 = self.indigo.loadMolecule("[O-]Cl.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains hypochlorite ion."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains hydroxide ion."
        )

    def test_check_salt_complex_secondary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[O-]I=O.C")
        m2 = self.indigo.loadMolecule("[O-]N(=O).C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains nitrite ion.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains iodite ion.")

    def test_check_salt_complex_tertiary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[N+](=O)([O-])[O-].C")
        m2 = self.indigo.loadMolecule("O[Se](=O)[O-].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains hydrogenselenite ion."
        )
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains nitrate ion.")

    def test_check_salt_complex_quaternary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("OP(=O)(O)[O-].C")
        m2 = self.indigo.loadMolecule("OS(=O)(=O)[O-].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains dihydrogenphosphate ion."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains dihydrogensulfate ion."
        )

    def test_check_salt_multiple_ions(self) -> None:
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

    def test_check_salt_no_ions(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("C1=CC=C2C=CC=CC2=C1")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_salt_bonded_metal_atom(self) -> None:
        m1 = self.indigo.loadMolecule("CC[Pb](CC)(CC)CC")
        m2 = self.indigo.loadMolecule("C[Al](C)C")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_salt_bonded_acid_group(self) -> None:
        m1 = self.indigo.loadMolecule("C1=CC=C(C=C1)[N+](=O)[O-]")
        m2 = self.indigo.loadMolecule("C(C(=O)O)S(=O)(=O)O")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_strip_salt_no_salts(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1")
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            (
                f"{m.smiles()} doesn't contain disconnected inorganic"
                " components."
            ),
        )

    def test_strip_salt_single_salt(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains [Cl-] anion.",
        )

    def test_strip_salt_many_salts(self) -> None:
        m = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.O.O.O.O.O.O.O.O.O.O.[Cl-].[Cl-]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains water molecules and [Cl-] anions.",
        )

    def test_strip_salt_two_organics(self) -> None:
        m = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "[O-]S(=O)(=O)[O-]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains [O-]S(=O)(=O)[O-] anion.",
        )

    def test_strip_salt_complex_salt(self) -> None:
        m = self.indigo.loadMolecule(
            "C(C(C(C(C(C(=O)O)O)O)O)O)O.C(C(C(C(C(C(=O)O)O)O)O)O)O.[Fe]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "C(O)C(O)C(O)C(O)C(O)C(O)=O.C(O)C(O)C(O)C(O)C(O)C(O)=O.[Fe]",
            f"{m.smiles()} doesn't contain disconnected inorganic components.",
        )

    def test_strip_salt_only_salts(self) -> None:
        m = self.indigo.loadMolecule("[NH4+].[O-]P(=O)([O-])[O-].[Fe+2]")
        self.assertEqual(
            m.stripSalt().smiles(),
            "",
            (
                f"{m.smiles()} contains [NH4+], [O-]P(=O)([O-])[O-] and [Fe+2]"
                " ions and no organic components."
            ),
        )

    def test_strip_salt_options(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
        m_strip = m.stripSalt()
        m.stripSalt(inplace=True)

        self.assertEqual(m_strip.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertNotEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.[Cl-]")
