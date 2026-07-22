from indigo import MyReaction
from tests import TestIndigoBase


class TestMyReaction(TestIndigoBase):
    def test_render_reaction_arrow_annotations(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        rxn = MyReaction(
            smiles="c1ccccc1Br.CBr.[Na].[Na]>>Cc1ccccc1.[Na]Br.[Na]Br",
            catalyst=["Na", "Dry Ether"],
            temperature="150",
            pressure="20",
            session=self.indigo,
        )
        svg = rxn.render_to_string()
        self.assertTrue(svg)
        self.assertIn("<svg", svg)
        self.assertIn("Na", svg)
        self.assertIn("Dry Ether", svg)
        self.assertIn("150", svg)
        self.assertIn("°C", svg)
        self.assertIn("20", svg)
        self.assertIn("psi", svg)
