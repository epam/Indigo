from tests import TestIndigoBase

from indigo.renderer import IndigoRenderer


class TestIndigoRenderer(TestIndigoBase):
    def setUp(self) -> None:
        super().setUp()
        self.indigo_renderer = IndigoRenderer(self.indigo)

    def test_render_svg(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        svg = self.indigo_renderer.renderToString(m)
        self.assertTrue(svg)
        self.assertIn("<svg", svg)
