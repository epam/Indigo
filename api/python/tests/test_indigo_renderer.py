import os
import re
import tempfile
import xml.etree.ElementTree as ET

from indigo.renderer import IndigoRenderer
from tests import TestIndigoBase


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

    def test_svg_ids_are_unique_between_renders(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        outputs = [
            self.indigo_renderer.renderToString(
                self.indigo.loadMolecule(smiles)
            )
            for smiles in ("N", "O")
        ]

        with tempfile.TemporaryDirectory() as directory:
            filename = os.path.join(directory, "oxygen.svg")
            self.indigo_renderer.renderToFile(
                self.indigo.loadMolecule("O"), filename
            )
            with open(filename, encoding="utf-8") as svg_file:
                outputs.append(svg_file.read())

        previous_ids = set()
        for svg in outputs:
            root = ET.fromstring(svg)
            ids = {
                element.attrib["id"]
                for element in root.iter()
                if "id" in element.attrib
            }
            references = set(re.findall(r"url\(#([^)]+)\)", svg))
            references.update(
                value[1:]
                for element in root.iter()
                for attribute, value in element.attrib.items()
                if attribute.endswith("href") and value.startswith("#")
            )

            self.assertTrue(ids)
            self.assertTrue(references.issubset(ids))
            self.assertTrue(ids.isdisjoint(previous_ids))
            previous_ids.update(ids)

    def test_svg_gradient_reference_is_prefixed(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        self.indigo.setOption("render-atom-color-property", "color")
        molecule = self.indigo.loadMolecule("NO")
        molecule.addDataSGroup([0], [], "color", "0.155, 0.55, 0.955")

        svg = self.indigo_renderer.renderToString(molecule)
        ids = {
            element.attrib["id"]
            for element in ET.fromstring(svg).iter()
            if "id" in element.attrib
        }
        gradient_references = re.findall(r"url\(#([^)]+)\)", svg)

        self.assertTrue(gradient_references)
        self.assertTrue(set(gradient_references).issubset(ids))
        self.assertTrue(
            all(
                reference.startswith("indigo-")
                for reference in gradient_references
            )
        )
