from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=CC=C1')
        m.aromatize()
        self.assertEqual('c1ccccc1', m.smiles())

    def test_check_single_ion(self) -> None:
        m1 = self.indigo.loadMolecule('[Na+].C')
        m2 = self.indigo.loadMolecule('[Ca+2].C')
        m3 = self.indigo.loadMolecule('[Al+3].C')
        m4 = self.indigo.loadMolecule('[Cl-].C')
        m5 = self.indigo.loadMolecule('[S-2].C')
        self.assertTrue(
            m1.checkSalt(), '{} contains Na+.'.format(m1.smiles())
        )
        self.assertTrue(
            m2.checkSalt(), '{} contains Ca2+.'.format(m1.smiles())
        )
        self.assertTrue(
            m3.checkSalt(), '{} contains Al3+.'.format(m3.smiles())
        )
        self.assertTrue(
            m4.checkSalt(), '{} contains Cl-.'.format(m4.smiles())
        )
        self.assertTrue(
            m5.checkSalt(), '{} contains S2-.'.format(m5.smiles())
        )

    def test_check_simple_molecular_salt(self) -> None:
        m1 = self.indigo.loadMolecule('OS(=O)(=O)O.C')
        m2 = self.indigo.loadMolecule('N.C')
        m3 = self.indigo.loadMolecule('O=[Si]=O.C')
        m4 = self.indigo.loadMolecule('S=[Fe].C')
        self.assertTrue(
            m1.checkSalt(), '{} contains sulfuric acid.'.format(m1.smiles())
        )
        self.assertTrue(
            m2.checkSalt(), '{} contains ammonia.'.format(m2.smiles())
        )
        self.assertTrue(
            m3.checkSalt(), '{} contains silicon dioxide.'.format(m3.smiles())
        )
        self.assertTrue(
            m4.checkSalt(), '{} contains ferrous sulfide.'.format(m4.smiles())
        )

    def test_check_complex_ion(self) -> None:
        m1 = self.indigo.loadMolecule('OP(=O)(O)[O-].C')
        m2 = self.indigo.loadMolecule('O[Se](=O)[O-].C')
        m3 = self.indigo.loadMolecule('[O-]N(=O).C')
        m4 = self.indigo.loadMolecule('[O-]Cl.C')
        self.assertTrue(
            m1.checkSalt(),
            '{} contains dihydrophosphate ion.'.format(m1.smiles())
        ),
        self.assertTrue(
            m2.checkSalt(),
            '{} contains hydrogenselenite ion.'.format(m2.smiles())
        )
        self.assertTrue(
            m3.checkSalt(),
            '{} contains nitrite ion.'.format(m3.smiles())
        )
        self.assertTrue(
            m4.checkSalt(),
            '{} contains hypochlorite ion.'.format(m4.smiles())
        )

    def test_check_multiple_ions(self) -> None:
        # TODO: update for counting matches
        m = self.indigo.loadMolecule('[Na+].[Cl-].C')
        self.assertTrue(
            m.checkSalt(),
            '{} contains sodium and chloride ions.'.format(m.smiles())
        )

    def test_check_no_ions(self) -> None:
        m = self.indigo.loadMolecule('c1ccccc1')
        self.assertFalse(
            m.checkSalt(),
            '{} doesn`t contain any salts.'.format(m.smiles())
        )

    def test_check_bonded_metal_atom(self) -> None:
        m = self.indigo.loadMolecule('CC[Pb](CC)(CC)CC')
        self.assertFalse(
            m.checkSalt(),
            '{} doesn`t contain any salts.'.format(m.smiles())
        )

    def test_check_bonded_nitro_group(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=C(C=C1)[N+](=O)[O-]')
        self.assertFalse(
            m.checkSalt(),
            '{} doesn`t contain any salts.'.format(m.smiles())
        )
