from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=CC=C1')
        m.aromatize()
        self.assertEqual('c1ccccc1', m.smiles())

    def test_similarity(self) -> None:
        m1 = self.indigo.loadMolecule('C1=CC=CC=C1')
        m2 = self.indigo.loadMolecule('C1=CC=CN=C1')
        sim = self.indigo.similarity(m1, m2, "tanimoto")
        self.assertGreater(sim, 0.0)
        self.assertLess(sim, 0.5)
