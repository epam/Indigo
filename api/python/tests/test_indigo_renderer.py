import os
import re
import tempfile
import xml.etree.ElementTree as ET

from indigo.renderer import IndigoRenderer
from tests import TestIndigoBase


def svg_ids_and_references(svg: str):
    root = ET.fromstring(svg)
    ids = [
        element.attrib["id"]
        for element in root.iter()
        if "id" in element.attrib
    ]
    references = set(re.findall(r"url\(#([^)]+)\)", svg))
    references.update(
        value[1:]
        for element in root.iter()
        for attribute, value in element.attrib.items()
        if attribute.endswith("href") and value.startswith("#")
    )
    return ids, references


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

    def assert_ids_resolve(self, svg: str) -> set:
        ids, references = svg_ids_and_references(svg)
        self.assertTrue(ids)
        self.assertEqual(len(ids), len(set(ids)))
        # An id starting with a digit breaks querySelector("#id") and CSS.
        self.assertTrue(all(i[0].isalpha() for i in ids))
        self.assertTrue(references)
        self.assertTrue(references.issubset(ids))
        return set(ids)

    def test_svg_ids_are_unique_between_renders(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        outputs = [
            self.indigo_renderer.renderToString(
                self.indigo.loadMolecule(smiles)
            )
            for smiles in ("N", "O", "O")
        ]

        with tempfile.TemporaryDirectory() as directory:
            filename = os.path.join(directory, "oxygen.svg")
            self.indigo_renderer.renderToFile(
                self.indigo.loadMolecule("O"), filename
            )
            with open(filename, encoding="utf-8") as svg_file:
                outputs.append(svg_file.read())

        previous_ids: set = set()
        for svg in outputs:
            ids = self.assert_ids_resolve(svg)
            self.assertTrue(ids.isdisjoint(previous_ids))
            previous_ids.update(ids)

    def test_svg_grid_ids_are_unique(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        grids = []
        for _ in range(2):
            molecules = self.indigo.createArray()
            for smiles in ("CCO", "c1ccccc1N", "CC(=O)O", "C1CC1Cl"):
                molecules.arrayAdd(self.indigo.loadMolecule(smiles))
            grids.append(
                bytes(
                    self.indigo_renderer.renderGridToBuffer(molecules, [], 2)
                ).decode("utf-8")
            )

        first = self.assert_ids_resolve(grids[0])
        second = self.assert_ids_resolve(grids[1])
        self.assertTrue(first.isdisjoint(second))

    def test_svg_gradient_reference_follows_unique_id(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        self.indigo.setOption("render-atom-color-property", "color")
        outputs = []
        for _ in range(2):
            molecule = self.indigo.loadMolecule("NO")
            molecule.addDataSGroup([0], [], "color", "0.155, 0.55, 0.955")
            outputs.append(self.indigo_renderer.renderToString(molecule))

        gradient_ids = []
        for svg in outputs:
            ids = self.assert_ids_resolve(svg)
            gradient_references = set(re.findall(r"url\(#([^)]+)\)", svg))
            self.assertTrue(gradient_references)
            self.assertTrue(gradient_references.issubset(ids))
            gradient_ids.append(gradient_references)
        self.assertTrue(gradient_ids[0].isdisjoint(gradient_ids[1]))

    def test_svg_unique_id_option(self) -> None:
        self.indigo.setOption("render-output-format", "svg")
        self.assertEqual(
            self.indigo.getOptionType("render-svg-unique-id"), "bool"
        )
        self.assertTrue(self.indigo.getOptionBool("render-svg-unique-id"))

        self.indigo.setOption("render-svg-unique-id", False)
        self.assertFalse(self.indigo.getOptionBool("render-svg-unique-id"))
        outputs = [
            self.indigo_renderer.renderToString(self.indigo.loadMolecule("O"))
            for _ in range(2)
        ]

        self.assertEqual(outputs[0], outputs[1])
        self.assertEqual(
            self.assert_ids_resolve(outputs[0]),
            self.assert_ids_resolve(outputs[1]),
        )

        self.indigo.resetOptions()
        self.assertTrue(self.indigo.getOptionBool("render-svg-unique-id"))

    def test_svg_not_well_formed_is_returned_unchanged(self) -> None:
        # With no custom fonts cairo may write an unterminated <image> for the
        # empty mask of an R-group label, so the SVG is not well-formed XML.
        # Prefixing ids cannot parse it and must leave the render intact.
        # Reproduced only on macOS; the test is skipped wherever cairo writes
        # well-formed SVG.
        # Smallest input that triggers it: a scaffold "C-R1" (R1 = N) in RGfile.
        rgroup_rgfile = "\n".join(
            [
                "$MDL  REV  1",
                "$MOL",
                "$HDR",
                "",
                "  test",
                "",
                "$END HDR",
                "$CTAB",
                "  2  1  0  0  0  0            999 V2000",
                "    0.0000    0.0000    0.0000 C   0  0",
                "    1.0000    0.0000    0.0000 R#  0  0",
                "  1  2  1  0  0  0  0",
                "M  LOG  1   1   1   1   1",
                "M  RGP  1   2   1",
                "M  END",
                "$END CTAB",
                "$RGP",
                "  1",
                "$CTAB",
                "  1  0  0  0  0  0            999 V2000",
                "    0.0000    0.0000    0.0000 N   0  0",
                "M  END",
                "$END CTAB",
                "$END RGP",
                "$END MOL",
                "",
            ]
        )
        self.indigo.setOption("render-output-format", "svg")
        self.indigo.setOption("render-fonts", "[]")
        molecule = self.indigo.loadMolecule(rgroup_rgfile)

        self.indigo.setOption("render-svg-unique-id", False)
        expected = self.indigo_renderer.renderToString(molecule)
        try:
            ET.fromstring(expected)
        except ET.ParseError:
            pass
        else:
            self.skipTest("cairo wrote well-formed SVG on this platform")

        self.indigo.setOption("render-svg-unique-id", True)
        self.assertEqual(
            self.indigo_renderer.renderToString(molecule), expected
        )
