#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json
import os
import unittest

import requests


# @unittest.skip("Skip libraries test case")
class IndigoTestCase(unittest.TestCase):
    def setUp(self):
        service_url = "http://front/v2"
        if (
            "INDIGO_SERVICE_URL" in os.environ
            and len(os.environ["INDIGO_SERVICE_URL"]) > 0
        ):
            service_url = os.environ["INDIGO_SERVICE_URL"]

        self.url_prefix = "{}/indigo".format(service_url)

    def tearDown(self):
        pass

    @staticmethod
    def get_headers(d):
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json",
        }
        data = json.dumps(d)
        headers["Content-Length"] = len(data)
        return headers, data

    formats = (
        "chemical/x-mdl-molfile",
        "chemical/x-daylight-smiles",
        "chemical/x-cml",
        "chemical/x-inchi",
    )

    aromatized_mols = {
        "chemical/x-mdl-molfile": (
            """
  -INDIGO-01000000002D

  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  4  0  0  0  0
  2  3  4  0  0  0  0
  3  4  4  0  0  0  0
  4  5  4  0  0  0  0
  5  6  4  0  0  0  0
  6  1  4  0  0  0  0
M  END
""",
            """
  -INDIGO-07261614562D

  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  4  0  0  0  0
  1  3  4  0  0  0  0
  2  4  4  0  0  0  0
  3  5  4  0  0  0  0
  4  6  4  0  0  0  0
  5  6  4  0  0  0  0
M  END
""",
        ),
        "chemical/x-daylight-smiles": ("c1ccccc1",),
        "chemical/x-cml": (
            """<?xml version="1.0" ?>
<cml>
    <molecule title="">
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="A" />
            <bond atomRefs2="a1 a2" order="A" />
            <bond atomRefs2="a2 a3" order="A" />
            <bond atomRefs2="a3 a4" order="A" />
            <bond atomRefs2="a4 a5" order="A" />
            <bond atomRefs2="a5 a0" order="A" />
        </bondArray>
    </molecule>
</cml>
""",
            """<?xml version="1.0" ?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="A" />
            <bond atomRefs2="a1 a2" order="A" />
            <bond atomRefs2="a2 a3" order="A" />
            <bond atomRefs2="a3 a4" order="A" />
            <bond atomRefs2="a4 a5" order="A" />
            <bond atomRefs2="a5 a0" order="A" />
        </bondArray>
    </molecule>
</cml>
""",
            """<?xml version="1.0" ?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="A" />
            <bond atomRefs2="a0 a2" order="A" />
            <bond atomRefs2="a1 a3" order="A" />
            <bond atomRefs2="a2 a4" order="A" />
            <bond atomRefs2="a3 a5" order="A" />
            <bond atomRefs2="a4 a5" order="A" />
        </bondArray>
    </molecule>
</cml>
""",
        ),
        "chemical/x-inchi": ("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H",),
    }

    dearomatized_mols = {
        "chemical/x-mdl-molfile": (
            """
  -INDIGO-01000000002D

  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3  4  2  0  0  0  0
  4  5  1  0  0  0  0
  5  6  2  0  0  0  0
  6  1  1  0  0  0  0
M  END
""",
            """
  -INDIGO-07261614382D

  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  2  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  6  1  2  0  0  0  0
M  END
""",
            """
  -INDIGO-07261614432D

  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  1  3  1  0  0  0  0
  2  4  1  0  0  0  0
  3  5  2  0  0  0  0
  4  6  2  0  0  0  0
  5  6  1  0  0  0  0
M  END
""",
        ),
        "chemical/x-daylight-smiles": ("C1C=CC=CC=1", "C1=CC=CC=C1"),
        "chemical/x-cml": (
            """<?xml version="1.0" ?>
<cml>
    <molecule title="">
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="2" />
            <bond atomRefs2="a1 a2" order="1" />
            <bond atomRefs2="a2 a3" order="2" />
            <bond atomRefs2="a3 a4" order="1" />
            <bond atomRefs2="a4 a5" order="2" />
            <bond atomRefs2="a5 a0" order="1" />
        </bondArray>
    </molecule>
</cml>
""",
            """<?xml version="1.0" ?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="1" />
            <bond atomRefs2="a1 a2" order="2" />
            <bond atomRefs2="a2 a3" order="1" />
            <bond atomRefs2="a3 a4" order="2" />
            <bond atomRefs2="a4 a5" order="1" />
            <bond atomRefs2="a5 a0" order="2" />
        </bondArray>
    </molecule>
</cml>
""",
            """<?xml version="1.0" ?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="2" />
            <bond atomRefs2="a0 a2" order="1" />
            <bond atomRefs2="a1 a3" order="1" />
            <bond atomRefs2="a2 a4" order="2" />
            <bond atomRefs2="a3 a5" order="2" />
            <bond atomRefs2="a4 a5" order="1" />
        </bondArray>
    </molecule>
</cml>
""",
            """<?xml version="1.0" ?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C" />
            <atom id="a1" elementType="C" />
            <atom id="a2" elementType="C" />
            <atom id="a3" elementType="C" />
            <atom id="a4" elementType="C" />
            <atom id="a5" elementType="C" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="2" />
            <bond atomRefs2="a1 a2" order="1" />
            <bond atomRefs2="a2 a3" order="2" />
            <bond atomRefs2="a3 a4" order="1" />
            <bond atomRefs2="a4 a5" order="2" />
            <bond atomRefs2="a5 a0" order="1" />
        </bondArray>
    </molecule>
</cml>
""",
        ),
        "chemical/x-inchi": ("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H",),
    }

    def test_info(self):
        headers, data = self.get_headers({})
        result = requests.get(
            self.url_prefix + "/info", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("Indigo", result_data)

    def test_aromatize_correct(self):
        formats = (
            "chemical/x-mdl-molfile",
            "chemical/x-daylight-smiles",
            "chemical/x-cml",
            "chemical/x-inchi",
        )
        for input_format in formats:
            for output_format in formats:
                result = requests.post(
                    self.url_prefix + "/aromatize",
                    headers={
                        "Content-Type": input_format,
                        "Accept": output_format,
                    },
                    data=self.dearomatized_mols[input_format][0],
                )
                self.assertEqual(200, result.status_code)
                self.assertEqual(output_format, result.headers["Content-Type"])
                if output_format in (
                    "chemical/x-mdl-molfile",
                    "chemical/x-mdl-rxnfile",
                ):  # Skip Molfile date
                    self.assertIn(
                        "\n".join(result.text.splitlines()[2:]).strip(),
                        [
                            "\n".join(m.splitlines()[2:]).strip()
                            for m in self.aromatized_mols[output_format]
                        ],
                    )
                else:
                    self.assertIn(
                        result.text, self.aromatized_mols[output_format]
                    )

    def test_aromatize_selected(self):
        headers, data = self.get_headers(
            {
                "struct": "C1C=CC=CC=1.C1C=CC=CC=1",
                "output_format": "chemical/x-daylight-smiles",
                "selected": [0, 1, 2, 3, 4, 5],
            }
        )
        result = requests.post(
            self.url_prefix + "/aromatize", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("c1ccccc1.c1ccccc1", result_data["struct"])

    def test_aromatize_selected_2(self):
        headers, data = self.get_headers(
            {
                "struct": "\n  Ketcher 10071619282D 1   1.00000     0.00000     0\n\n 13 13  0     0  0            999 V2000\n    0.0000    0.8660    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n    0.9996    0.8661    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    1.4996    1.7321    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    2.4996    1.7322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    2.9997    0.8661    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    2.4997    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    1.4997    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.8651    2.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    4.9990    1.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    4.9990    0.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.8650    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    6.7311    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    6.7311    1.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n  1  2  1  0     0  0\n  2  3  4  0     0  0\n  3  4  4  0     0  0\n  4  5  4  0     0  0\n  5  6  4  0     0  0\n  6  7  4  0     0  0\n  7  2  4  0     0  0\n  8  9  2  0     0  0\n  9 10  1  0     0  0\n 10 11  2  0     0  0\n 11 12  1  0     0  0\n 12 13  2  0     0  0\n 13  8  1  0     0  0\nM  END\n",
                "selected": [7, 8, 9, 10, 11, 12],
                "output_format": "chemical/x-daylight-smiles",
                "options": {
                    "smart-layout": True,
                    "ignore-stereochemistry-errors": True,
                },
            }
        )
        result = requests.post(
            self.url_prefix + "/aromatize", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(200, result.status_code)
        self.assertEqual("Nc1ccccc1.c1ccccc1", result_data["struct"])

    def test_smiles_wrong(self):
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-daylight-smiles",
            },
            data="c1ccccc2",
        )
        self.assertEqual(400, result.status_code)
        self.assertEqual(
            "IndigoException: molecule auto loader: SMILES loader: unexpected end of input",
            result.text,
        )

    def test_headers_is_rxn(self):
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={"Content-Type": "chemical/x-daylight-smiles"},
            data="CC",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            "chemical/x-mdl-molfile", result.headers["Content-Type"]
        )
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={"Content-Type": "chemical/x-daylight-smiles"},
            data="CC>>CC",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            "chemical/x-mdl-rxnfile", result.headers["Content-Type"]
        )

    def test_headers_wrong(self):
        # Missing both Content-Type and Accept headers
        result = requests.post(
            self.url_prefix + "/aromatize", headers={}, data="c1ccccc1"
        )
        self.assertEqual(400, result.status_code)
        self.assertIn("'input_format': ['Not a valid choice.']", result.text)
        # Missing Accept header
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={"Content-Type": "chemical/x-daylight-smiles"},
            data="c1ccccc1",
        )
        self.assertEqual(200, result.status_code)
        # Wrong Content-Type header
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={
                "Content-Type": "chemical/x-daylight-smiles1",
                "Accept": "chemical/x-daylight-smiles",
            },
            data="c1ccccc1",
        )
        self.assertEqual(400, result.status_code)
        self.assertEqual(
            "ValidationError: {'input_format': ['Not a valid choice.']}",
            result.text,
        )
        # Wrong Accept header
        result = requests.post(
            self.url_prefix + "/aromatize",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-daylight-smiles2",
            },
            data="c1ccccc1",
        )
        self.assertEqual(400, result.status_code)
        self.assertEqual(
            "ValidationError: {'output_format': ['Not a valid choice.']}",
            result.text,
        )

    def test_dearomatize_correct(self):
        formats = (
            "chemical/x-mdl-molfile",
            "chemical/x-daylight-smiles",
            "chemical/x-cml",
            "chemical/x-inchi",
        )
        for input_format in formats:
            for output_format in formats:
                result = requests.post(
                    self.url_prefix + "/dearomatize",
                    headers={
                        "Content-Type": input_format,
                        "Accept": output_format,
                    },
                    data=self.aromatized_mols[input_format][0],
                )
                self.assertEqual(200, result.status_code)
                self.assertEqual(output_format, result.headers["Content-Type"])
                if output_format in (
                    "chemical/x-mdl-molfile",
                    "chemical/x-mdl-rxnfile",
                ):  # Skip Molfile date
                    self.assertIn(
                        "\n".join(result.text.splitlines()[2:]),
                        [
                            "\n".join(m.splitlines()[2:])
                            for m in self.dearomatized_mols[output_format]
                        ],
                    )
                else:
                    self.assertIn(
                        result.text, self.dearomatized_mols[output_format]
                    )

    def test_dearomatize_selected(self):
        headers, data = self.get_headers(
            {
                "struct": "c1ccccc1.c1ccccc1",
                "output_format": "chemical/x-daylight-smiles",
                "selected": [0, 1, 2, 3, 4, 5],
            }
        )
        result = requests.post(
            self.url_prefix + "/dearomatize", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C1C=CC=CC=1.C1C=CC=CC=1", result_data["struct"])

    def test_dearomatize_query_molecule(self):
        result = requests.post(
            self.url_prefix + "/dearomatize",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-daylight-smiles",
            },
            data="CX",
        )
        self.assertEqual(400, result.status_code)
        self.assertEqual(
            "Structures with query features cannot be dearomatized yet",
            result.text,
        )

    def test_convert_correct(self):
        formats = (
            "chemical/x-mdl-molfile",
            "chemical/x-daylight-smiles",
            "chemical/x-cml",
            "chemical/x-inchi",
        )
        # Test for POST request
        for input_format in formats:
            for output_format in formats:
                result = requests.post(
                    self.url_prefix + "/convert",
                    headers={
                        "Content-Type": input_format,
                        "Accept": output_format,
                    },
                    data=self.dearomatized_mols[input_format][0],
                )
                self.assertEqual(200, result.status_code)
                self.assertEqual(output_format, result.headers["Content-Type"])
                if output_format in (
                    "chemical/x-mdl-molfile",
                    "chemical/x-mdl-rxnfile",
                ):  # Skip Molfile date
                    self.assertIn(
                        "\n".join(result.text.splitlines()[2:]),
                        [
                            "\n".join(m.splitlines()[2:])
                            for m in self.dearomatized_mols[output_format]
                        ],
                    )
                else:
                    self.assertIn(
                        result.text, self.dearomatized_mols[output_format]
                    )

            for output_format in formats:
                # Test for GET request
                result = requests.get(
                    self.url_prefix + "/convert",
                    params={
                        "struct": self.dearomatized_mols[input_format][0],
                        "output_format": output_format,
                    },
                )
                self.assertEqual(200, result.status_code)
                if output_format in (
                    "chemical/x-mdl-molfile",
                    "chemical/x-mdl-rxnfile",
                ):  # Skip Molfile date
                    self.assertIn(
                        "\n".join(result.text.splitlines()[2:]),
                        [
                            "\n".join(m.splitlines()[2:])
                            for m in self.dearomatized_mols[output_format]
                        ],
                    )
                else:
                    self.assertIn(
                        result.text, self.dearomatized_mols[output_format]
                    )

    def test_convert_canonical_smiles(self):
        headers, data = self.get_headers(
            {
                "struct": "C1=CC=CC=C1O",
                "output_format": "chemical/x-daylight-smiles",
            }
        )
        result_standard = requests.post(
            self.url_prefix + "/convert", headers=headers, data=data
        )
        headers, data = self.get_headers(
            {
                "struct": "C1=CC=CC=C1O",
                "output_format": "chemical/x-daylight-smiles",
                "options": {"smiles": "canonical"},
            }
        )
        result_canonical = requests.post(
            self.url_prefix + "/convert", headers=headers, data=data
        )
        self.assertNotEqual(
            json.loads(result_canonical.text)["struct"],
            json.loads(result_standard.text)["struct"],
        )

    def test_convert_smarts(self):
        smarts = [
            "[#8;A]-[!#1]-[#6;A](-[#9])(-[#9])-[#9]",
            "[#6,#1]",
            "[#1,#1]",
            "[#9,#17,#35,#53,#7&A&+,$([OH]-*=[!#6]),+;!#1]",
        ]
        results = []
        results_get = []
        for mol in smarts:
            params = {
                "struct": mol,
                "input_format": "chemical/x-daylight-smarts",
                "output_format": "chemical/x-daylight-smarts",
            }
            headers, data = self.get_headers(params)
            result = requests.post(
                self.url_prefix + "/convert", headers=headers, data=data
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            results.append(result_data["struct"])

            result = requests.get(self.url_prefix + "/convert", params=params)
            self.assertEqual(200, result.status_code)
            results_get.append(result.text)

        self.assertEqual(smarts, results)
        self.assertEqual(smarts, results_get)

    def test_convert_name_to_structure(self):
        names = [
            "methane",
            "ethane",
            "propane",
            "butane",
            "ethene",
            "propene",
            "butene",
            "ethyne",
            "propyne",
            "butyne",
            "oct-3-ene",
            "oct-5,3-diene",
            "oct-3-yne",
            "oct-3,5-diyne",
            "3-ethyl-octane",
            "3,5-diethyl-octane",
            "3-methyl-5-ethyl-octane",
            "3-(2,4-dimethyl-pentyl)-octane",
            "3-methyl-5-ethyl-octane",
            "3-(2,4-dimethyl-pentyl)-octane",
            "cyclooctane",
            "cyclooctene",
            "cyclooctyne",
            "3-methyl-5-ethyl-cyclooctane",
            "cyclotetradecane",
            "cyclododeca-1,3,5,7,9,11-hexaene",
        ]
        smiles = [
            "C",
            "CC",
            "CCC",
            "CCCC",
            "C=C",
            "C=CC",
            "C=CCC",
            "C#C",
            "C#CC",
            "C#CCC",
            "CCC=CCCCC",
            "CCC=CC=CCC",
            "CCC#CCCCC",
            "CCC#CC#CCC",
            "CCC(CCCCC)CC",
            "CCC(CC(CCC)CC)CC",
            "CCC(CC(CCC)CC)C",
            "CCC(CCCCC)CC(CC(C)C)C",
            "CCC(CC(CCC)CC)C",
            "CCC(CCCCC)CC(CC(C)C)C",
            "C1CCCCCCC1",
            "C1CCCCCCC=1",
            "C1CCCCCCC#1",
            "C1CCCC(CC)CC(C)C1",
            "C1CCCCCCCCCCCCC1",
            "C1C=CC=CC=CC=CC=CC=1",
        ]
        results = []
        results_get = []
        for name in names:
            params = {
                "struct": name,
                "input_format": "chemical/x-iupac",
                "output_format": "chemical/x-daylight-smiles",
            }

            # POST
            headers, data = self.get_headers(params)
            result = requests.post(
                self.url_prefix + "/convert", headers=headers, data=data
            )
            self.assertEqual(200, result.status_code)
            result_data = json.loads(result.text)
            results.append(result_data["struct"])

            # GET
            result = requests.get(self.url_prefix + "/convert", params=params)
            results_get.append(result.text)
        self.assertEqual(smiles, results)
        self.assertEqual(smiles, results_get)

    def test_convert_utf8(self):

        text = """
  Ketcher 02051318482D 1   1.00000     0.00000     0

  5  4  0     0  0            999 V2000
   -4.1250   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.2590   -8.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.3929   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.5269   -8.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.6609   -8.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
M  STY  1   1 DAT
M  SLB  1   1   1
M  SAL   1  2   4   5
M  SDT   1 single-name                    F
M  SDD   1     1.6314   -1.1000    DR    ALL  1      1
M  SED   1 single-value-бензол
M  END
"""

        answ = """$MDL  REV  1
$MOL
$HDR



$END HDR
$CTAB
 15 14  0     0  0            999 V2000
    6.6250   -7.3500    0.0000 R#  0  0  0  0  0  0  0  0  0  0  0  0
    7.4910   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.3571   -7.3500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.2231   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.0891   -7.3500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.9551   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.8212   -7.3500    0.0000 C   0  0  0  4  0  0  0  0  0  0  0  0
   12.6872   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.5532   -7.3500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.4192   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.2853   -7.3500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.1513   -7.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  1
   17.0173   -7.3500    0.0000 C   0  0  0  0  0  0  0  0  0  0  1  0
   17.8833   -7.8500    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0
   15.2853   -6.3500    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  1
  5  6  1  0     0  0
  6  7  1  0     1  0
  7  8  1  0     0  0
  8  9  1  0     2  0
  9 10  1  0     0  0
 10 11  1  0     0  0
 11 12  1  0     0  0
 12 13  1  0     0  0
 13 14  1  0     0  0
 11 15  1  0     0  0
M  RGP  1   1   1
M  RBC  1   5  -2
M  SUB  1   9  -2
M  UNS  1   3   1
M  ALS  14  3 F B   Si  As
M  ALS  15  3 T N   P   As
M  END
$END CTAB
$RGP
  1
$CTAB
  6  6  0     0  0            999 V2000
    7.9000  -10.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.7660  -11.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.7660  -12.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.9000  -12.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.0340  -12.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.0340  -11.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  END
$END CTAB
$END RGP
$END MOL
"""

        # POST
        result = requests.post(
            self.url_prefix + "/convert",
            headers={
                "Content-Type": "chemical/x-mdl-molfile",
                "Accept": "chemical/x-cml",
            },
            data=text.encode("utf-8"),
        )
        self.assertEqual(200, result.status_code)
        result = requests.post(
            self.url_prefix + "/convert",
            headers={
                "Content-Type": "chemical/x-mdl-molfile",
                "Accept": "chemical/x-cml",
            },
            data=answ,
        )
        self.assertEqual(200, result.status_code)

        # GET
        result = requests.get(
            self.url_prefix + "/convert",
            params={"struct": text.encode("utf-8")},
        )
        self.assertEqual(200, result.status_code)
        result = requests.get(
            self.url_prefix + "/convert", params={"struct": answ}
        )
        self.assertEqual(200, result.status_code)

    def test_layout(self):
        result = requests.post(
            self.url_prefix + "/layout",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-mdl-molfile",
            },
            data="C1=CC=CC=C1",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            """
  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.3856   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.3856   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -3.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3856   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3856   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3  4  2  0  0  0  0
  4  5  1  0  0  0  0
  5  6  2  0  0  0  0
  6  1  1  0  0  0  0
M  END""",
            "\n".join(result.text.splitlines()[2:]),
        )

    def test_layout_selective(self):
        headers, data = self.get_headers(
            {
                "struct": "CCC",
                "output_format": "chemical/x-mdl-molfile",
                "selected": [1, 2],
            }
        )
        result = requests.post(
            self.url_prefix + "/layout", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            """
  3  2  0  0  0  0  0  0  0  0999 V2000
   -1.3856    0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3856    0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
M  END""",
            "\n".join(result_data["struct"].splitlines()[2:]),
        )

    def test_layout_selective_reaction(self):
        headers, data = self.get_headers(
            {
                "struct": """$RXN



  2  1  0
$MOL

  Ketcher 10071615322D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
    0.5450    0.6292    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0001    0.3146    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -0.3146    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5449   -0.6292    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0898   -0.3146    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0898    0.3146    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  END
$MOL

  Ketcher 10071615322D 1   1.00000     0.00000     0

 12 13  0     0  0            999 V2000
    3.0898   -0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.7135    1.0801    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.9608    1.0803    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5845    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.9609   -1.0803    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3636   -3.8303    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8313    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3519    0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.3931    0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.9137    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0931   -4.9516    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3519   -0.9016    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
  4  7  1  0     0  0
  7  8  1  0     0  0
  8  9  2  0     0  0
  9 10  1  0     0  0
 10 11  2  0     0  0
 11 12  1  0     0  0
 12  7  2  0     0  0
M  END
$MOL

  Ketcher 10071615322D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   16.4754    0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4343    0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.9137    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4343   -0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.4754   -0.9017    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.9960    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  END
""",
                "selected": [5, 6],
                "output_format": "chemical/x-mdl-rxnfile",
                "options": {"molfile-saving-skip-date": "1"},
            }
        )
        result = requests.post(
            self.url_prefix + "/layout", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            """$RXN

 -INDIGO- 0100000000

  2  1
$MOL

  -INDIGO-01000000002D

  6  6  0  0  0  0  0  0  0  0999 V2000
    1.3856    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3856   -3.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7713   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7713   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  2  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  6  1  2  0  0  0  0
M  END
$MOL

  -INDIGO-01000000002D

 12 13  0  0  0  0  0  0  0  0999 V2000
    8.2513   -1.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0513   -0.2144    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.6513   -0.2144    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.4513   -1.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.6513   -2.9856    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0513   -2.9856    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.0513   -1.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.8513   -0.2144    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4513   -0.2144    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.2513   -1.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4513   -2.9856    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.8513   -2.9856    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  2  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  6  1  2  0  0  0  0
  4  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  2  0  0  0  0
  9 10  1  0  0  0  0
 10 11  2  0  0  0  0
 11 12  1  0  0  0  0
 12  7  2  0  0  0  0
M  END
$MOL

  -INDIGO-01000000002D

  6  6  0  0  0  0  0  0  0  0999 V2000
   23.1169    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.7313   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.7313   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.1169   -3.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   24.5026   -2.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   24.5026   -0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  2  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  6  1  2  0  0  0  0
M  END
""",
            result_data["struct"],
        )

    def test_clean(self):
        result = requests.post(
            self.url_prefix + "/clean",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-mdl-molfile",
            },
            data="C1=CC=CC=C1",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual(
            """
  6  6  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3  4  2  0  0  0  0
  4  5  1  0  0  0  0
  5  6  2  0  0  0  0
  6  1  1  0  0  0  0
M  END""",
            "\n".join(result.text.splitlines()[2:]),
        )

    def test_automap_no_header(self):
        result = requests.post(
            self.url_prefix + "/automap",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-daylight-smiles",
            },
            data="C>>C",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("[CH4:1]>>[CH4:1]", result.text)

    def test_automap_correct_header(self):
        result = requests.post(
            self.url_prefix + "/automap",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "chemical/x-daylight-smiles",
            },
            data="C>>C",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("[CH4:1]>>[CH4:1]", result.text)
        headers, data = self.get_headers(
            {
                "struct": "C>>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "discard",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("[CH4:1]>>[CH4:1]", result_data["struct"])
        headers, data = self.get_headers(
            {
                "struct": "C>>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "keep",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("[CH4:1]>>[CH4:1]", result_data["struct"])
        headers, data = self.get_headers(
            {
                "struct": "C>>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "alter",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("[CH4:1]>>[CH4:1]", result_data["struct"])
        headers, data = self.get_headers(
            {
                "struct": "C>>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "clear",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C>>C", result_data["struct"])

    def test_automap_wrong_header(self):
        headers, data = self.get_headers(
            {
                "struct": "C>>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "wrong_mode",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Not a valid choice.", "".join(result_data["error"]["mode"])
        )

    def test_automap_wrong_reaction(self):
        headers, data = self.get_headers(
            {
                "struct": "C>C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "discard",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "IndigoException: molecule auto loader: SMILES loader: invalid character within atom description: '>'",
            result_data["error"],
        )

    def test_automap_molecule_instead_of_reaction(self):
        headers, data = self.get_headers(
            {
                "struct": "C",
                "output_format": "chemical/x-daylight-smiles",
                "mode": "discard",
            }
        )
        result = requests.post(
            self.url_prefix + "/automap", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "IndigoException: core: <molecule> is not a base reaction",
            result_data["error"],
        )

    def test_calculate_cip_correct(self):
        result = requests.post(
            self.url_prefix + "/calculate_cip",
            headers={
                "Content-Type": "chemical/x-mdl-molfile",
                "Accept": "chemical/x-mdl-molfile",
            },
            data="""
  Ketcher 07261618302D 1   1.00000     0.00000     0

 12 12  0     0  0            999 V2000
    9.3770  -13.9546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -13.4548    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -12.4552    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -13.9546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -13.4548    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -12.4552    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -11.9546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -14.9550    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -15.4548    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -11.4546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -10.9548    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    9.2427  -12.4552    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0     0  0
  2  3  1  0     0  0
  3 10  1  1     0  0
  3  7  1  0     0  0
  7 11  1  6     0  0
  2  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  4  8  1  0     0  0
  8  9  1  0     0  0
  3 12  1  0     0  0
M  END
""",
        )
        self.assertEqual(200, result.status_code)
        # print(result.text)
        self.assertEqual(
            """
 12 12  0  0  0  0  0  0  0  0999 V2000
    9.3770  -13.9546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -13.4548    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -12.4552    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0
   11.1091  -13.9546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -13.4548    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -12.4552    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -11.9546    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0
   11.1091  -14.9550    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9749  -15.4548    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.2427  -11.4546    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1091  -10.9548    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    9.2427  -12.4552    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  3 10  1  1  0  0  0
  3  7  1  0  0  0  0
  7 11  1  6  0  0  0
  2  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  4  8  1  0  0  0  0
  8  9  1  0  0  0  0
  3 12  1  0  0  0  0
M  STY  2   1 DAT   2 DAT
M  SLB  2   1   1   2   2
M  SAL   1  1   3
M  SDT   1 INDIGO_CIP_DESC
M  SDD   1     0.0000    0.0000    DR    ALL  1       1
M  SED   1 (s)
M  SAL   2  1   7
M  SDT   2 INDIGO_CIP_DESC
M  SDD   2     0.0000    0.0000    DR    ALL  1       1
M  SED   2 (R)
M  END""",
            "\n".join([s.rstrip() for s in result.text.splitlines()[2:]]),
        )

    def test_render(self):
        result = requests.post(
            self.url_prefix + "/render",
            headers={
                "Content-Type": "chemical/x-daylight-smiles",
                "Accept": "image/svg+xml",
            },
            data="C",
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("image/svg+xml", result.headers["Content-Type"])
        headers, data = self.get_headers({"struct": "C"})
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("image/svg+xml", result.headers["Content-Type"])
        headers, data = self.get_headers(
            {"struct": "C", "output_format": "application/pdf"}
        )
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("application/pdf", result.headers["Content-Type"])
        headers, data = self.get_headers(
            {"struct": "C", "output_format": "image/png"}
        )
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("image/png", result.headers["Content-Type"])

        # GET
        data = {"struct": "c1ccccc1", "output_format": "image/png"}
        result = requests.get(self.url_prefix + "/render", params=data)
        self.assertEqual(200, result.status_code)

        data = {"struct": "c1ccccc1", "output_format": "image/svg+xml"}
        result = requests.get(self.url_prefix + "/render", params=data)
        self.assertEqual(200, result.status_code)

        data = {"struct": "c1ccccc1", "output_format": "application/pdf"}
        result = requests.get(self.url_prefix + "/render", params=data)
        self.assertEqual(200, result.status_code)

    def test_renderhighlight(self):
        params = {"struct": "C1=CC=CC=C1", "query": "C"}
        headers, data = self.get_headers(params)
        # POST
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        self.assertEqual("image/svg+xml", result.headers["Content-Type"])
        # GET
        result = requests.get(self.url_prefix + "/render", params=params)
        self.assertEqual(200, result.status_code)

    def test_render_exceptions(self):
        # either query or structure should be present
        headers, data = self.get_headers({})
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("_schema", result_data["error"])
        # GET
        result = requests.get(self.url_prefix + "/render", params={})
        self.assertEqual(400, result.status_code)
        # render format is wrong
        headers, data = self.get_headers(
            {"struct": "C", "output_format": "foo"}
        )
        result = requests.post(
            self.url_prefix + "/render", headers=headers, data=data
        )
        self.assertEqual(400, result.status_code)
        result_data = json.loads(result.text)
        self.assertIn("output_format", result_data["error"])
        # GET
        result = requests.get(
            self.url_prefix + "/render",
            params={"struct": "C", "output_format": "foo"},
        )
        self.assertEqual(400, result.status_code)

    def test_json_aromatize_correct(self):
        formats = (
            "chemical/x-mdl-molfile",
            "chemical/x-daylight-smiles",
            "chemical/x-cml",
            "chemical/x-inchi",
        )
        for input_format in formats:
            for output_format in formats:
                headers, data = self.get_headers(
                    {
                        "struct": self.dearomatized_mols[input_format][0],
                        "input_format": input_format,
                        "output_format": output_format,
                    }
                )
                result = requests.post(
                    self.url_prefix + "/aromatize", headers=headers, data=data
                )
                self.assertEqual(200, result.status_code)
                result_data = json.loads(result.text)
                result_struct = result_data["struct"]
                result_format = result_data["format"]
                self.assertEqual(result_format, output_format)
                if output_format in (
                    "chemical/x-mdl-molfile",
                    "chemical/x-mdl-rxnfile",
                ):  # Skip Molfile date
                    self.assertIn(
                        "\n".join(result_struct.splitlines()[2:]).strip(),
                        [
                            "\n".join(m.splitlines()[2:]).strip()
                            for m in self.aromatized_mols[output_format]
                        ],
                    )
                else:
                    self.assertIn(
                        result_struct, self.aromatized_mols[output_format]
                    )

    def test_json_check(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 08151618402D 1   1.00000     0.00000     0

 13 13  0     0  0            999 V2000
   -0.8662    1.5003    0.0000 C   0  0  0  0  0  7  0  0  0  0  0  0
   -1.7324    1.0003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7324    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.8662   -0.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0001    1.0002    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5982    1.5002    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8659    1.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.8663   -1.4999    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    0.5876   -0.8089    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.9943   -0.1045    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.2079   -0.9779    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.7431    0.6690    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  1  6  2  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  2  7  1  0     0  0
  6  8  1  0     0  0
  4  9  1  0     0  0
  5 10  1  0     0  0
  5 11  1  0     0  0
  5 12  1  0     0  0
  5 13  1  0     0  0
M  CHG  1   3 -40
M  RAD  1   3   3
M  STY  1   1 DAT
M  SLB  1   1   1
M  SAL   1  1   7
M  SDT   1 INDIGO_ALIAS
M  SDD   1    -2.5982    1.5002    AA    ALL  1      1
M  SED   1 Psd
M  STY  1   2 DAT
M  SLB  1   2   2
M  SAL   2  1   8
M  SDT   2 INDIGO_ALIAS
M  SDD   2     0.8659    1.5001    AA    ALL  1      1
M  SED   2 Pol
M  END
""",
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains 2 atoms with bad valence",
            result_data["valence"],
        )
        self.assertEqual(
            "Structure contains 1 atom with radical electrons",
            result_data["radicals"],
        )
        self.assertEqual("Structure has SGroups", result_data["sgroups"])

    def test_check(self):
        result = requests.post(
            self.url_prefix + "/check",
            headers={"Content-Type": "chemical/x-mdl-molfile"},
            data="""
  Ketcher 08121615592D 1   1.00000     0.00000     0

 13 12  0     0  0            999 V2000
   22.2500  -10.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.2500  -10.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.7500  -10.0090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.7500  -11.7410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.2500  -10.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.7500  -10.0090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.7500  -11.7410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.1160  -10.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.1160  -11.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3840  -11.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3840  -10.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.2500   -9.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.2500  -11.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  1  3  1  0     0  0
  1  4  1  0     0  0
  1  5  1  0     0  0
  1  6  1  0     0  0
  1  7  1  0     0  0
  1  8  1  0     0  0
  1  9  1  0     0  0
  1 10  1  0     0  0
  1 11  1  0     0  0
  1 12  1  0     0  0
  1 13  1  0     0  0
M  END
""",
        )
        self.assertEqual(200, result.status_code)
        result_data = result.text
        self.assertEqual(
            "valence: Structure contains 1 atom with bad valence", result_data
        )

    def test_check_overlap(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 08221617222D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
    2.5908  -10.5562    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.4568  -11.0562    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.4568  -12.0563    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.5908  -12.5563    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.7248  -11.0563    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.7248  -11.0562    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  6  1  2  0     0  0
M  END

""",
                "types": ["overlapping_atoms"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains overlapping atoms",
            result_data["overlapping_atoms"],
        )

    def test_check_stereo(self):
        # up
        headers, data = self.get_headers(
            {
                "struct": """
  -INDIGO-10201021542D

 10 11  0  0  0  0  0  0  0  0999 V2000
   10.2958   -8.7041    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.9326   -8.7062    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9333   -7.9249    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.9500   -9.5749    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.2625   -7.8874    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.2625   -9.6499    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.5541   -7.1333    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.5750  -10.3708    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.2625   -6.9041    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.4708  -10.7124    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  4  2  1  0  0  0  0
  5  1  1  0  0  0  0
  6  1  1  0  0  0  0
  3  4  1  0  0  0  0
  5  6  1  0  0  0  0
  7  3  1  1  0  0  0
  4  8  1  1  0  0  0
  5  9  1  1  0  0  0
 10  6  1  1  0  0  0
  1  2  2  0  0  0  0
  3  2  1  0  0  0  0
M  END

""",
                "types": [
                    "stereo",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure has stereochemistry errors", result_data["stereo"]
        )
        # cis
        headers, data = self.get_headers(
            {
                "struct": r"F/C=C\F",
                "types": [
                    "stereo",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)
        # trans
        headers, data = self.get_headers(
            {
                "struct": "F/C=C/F",
                "types": [
                    "stereo",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)
        # normal
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 10171617062D 1   1.00000     0.00000     0

  6  6  0     0  0            999 V2000
   20.5253   -4.7001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3913   -5.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.3913   -6.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.5253   -6.7001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.6593   -6.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.6593   -5.2001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  5  6  2  0     0  0
  2  3  1  0     0  0
  3  4  2  0     0  0
  4  5  1  0     0  0
  6  1  1  0     0  0
  1  2  2  0     0  0
M  END
""",
                "types": [
                    "stereo",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)

    def test_check_query(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09201617322D 1   1.00000     0.00000     0

 27 27  0     0  0            999 V2000
    5.4529   -1.9710    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.4249   -4.3553    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.5034   -8.4509    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8029  -12.0523    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.6382   -2.9124    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7353   -2.5317    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.9475   -2.9655    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.7705   -4.0012    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8167   -4.4748    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5503   -3.0363    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5503   -3.9835    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.3559   -5.5946    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5503   -6.2408    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5326   -7.2234    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5478   -7.7943    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.7527   -6.2231    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.8944   -7.2234    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7707   -7.6925    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.6737   -7.3119    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.8333   -8.7637    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8778   -9.0646    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.1346  -10.0650    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.1968  -10.4102    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.4623  -11.3929    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.3425   -4.6034    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5399   -2.4419    0.0000 C   0  0  0  0  0  3  0  0  0  0  0  0
    0.9503   -5.7709    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
  5  6  1  0     0  0
  6  7  2  0     0  0
  7  8  1  0     0  0
  8  9  2  0     0  0
  2  9  1  0     0  0
 10 11  2  0     0  0
 12 13  1  0     0  0
 13 14  2  0     0  0
 14 15  1  0     0  0
 16 17  1  0     0  0
 17 18  2  0     0  0
 18 19  1  0     0  0
 15 19  2  0     0  0
 15 20  1  0     0  0
 20 21  1  0     0  0
  3 21  1  0     0  0
 21 22  1  0     0  0
 22 23  1  0     0  0
 23 24  1  0     0  0
  4 24  1  0     0  0
 11 25  1  0     0  0
 12 25  2  0     0  0
  1 26  1  0     0  0
  5 26  2  0     0  0
 10 26  1  0     0  0
  9 27  1  0     0  0
 16 27  2  0     0  0
M  END
""",
                "types": [
                    "valence",
                    "ambiguous_h",
                    "query",
                    "pseudoatoms",
                    "radicals",
                    "stereo",
                    "overlapping_atoms",
                    "overlapping_bonds",
                    "3d",
                    "sgroups",
                    "v3000",
                    "rgroups",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains query features, so valency could not be checked",
            result_data["valence"],
        )
        self.assertEqual(
            "Structure contains query features, so ambiguous H could not be checked",
            result_data["ambiguous_h"],
        )
        self.assertEqual(
            "Structure contains query features", result_data["query"]
        )

    def test_check_pseudoatom(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Marvin  02121015302D

  9  9  0  0  0  0            999 V2000
   -1.8857    2.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.6002    2.0625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.6002    1.2375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.8857    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1712    1.2375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1712    2.0625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.3147    2.4750    0.0000 Psd 0  0  0  0  0  0  0  0  0  0  0  0
   -0.4568    2.4750    0.0000 Pol 0  0  0  0  0  0  0  0  0  0  0  0
   -1.8857   -0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  1  6  2  0  0  0  0
  2  3  2  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  5  6  1  0  0  0  0
  2  7  1  0  0  0  0
  6  8  1  0  0  0  0
  4  9  1  0  0  0  0
M  END
""",
                "types": [
                    "valence",
                    "ambiguous_h",
                    "query",
                    "pseudoatoms",
                    "radicals",
                    "stereo",
                    "overlapping_atoms",
                    "overlapping_bonds",
                    "3d",
                    "sgroups",
                    "v3000",
                    "rgroups",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains pseudoatoms, so radicals could not be checked",
            result_data["radicals"],
        )
        self.assertEqual(
            "Structure contains 2 pseudoatoms", result_data["pseudoatoms"]
        )

    def test_check_empty(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09221617072D 1   1.00000     0.00000     0

  0  0  0     0  0            999 V2000
M  END
""",
                "types": [
                    "valence",
                    "ambiguous_h",
                    "query",
                    "pseudoatoms",
                    "radicals",
                    "stereo",
                    "overlapping_atoms",
                    "overlapping_bonds",
                    "3d",
                    "sgroups",
                    "v3000",
                    "rgroups",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)

    def test_check_overlapping_bonds(self):
        # intersecting bonds
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09271616302D 1   1.00000     0.00000     0

  4  2  0     0  0            999 V2000
   -0.3081   -0.0005    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.6917   -0.0225    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1818   -0.5124    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.2038    0.4874    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  3  4  1  0     0  0
M  END
""",
                "types": ["overlapping_bonds"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains overlapping bonds",
            result_data["overlapping_bonds"],
        )
        # two bonds from one atom:
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09271617112D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.8660    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8660    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  1  3  1  0     0  0
M  END
""",
                "types": ["overlapping_bonds"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)
        # bonds on the same line
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09271617122D 1   1.00000     0.00000     0

  4  2  0     0  0            999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  3  4  1  0     0  0
M  END
""",
                "types": ["overlapping_bonds"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)
        # parallel bonds
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 09271617122D 1   1.00000     0.00000     0

  4  2  0     0  0            999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    1.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    1.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  3  4  1  0     0  0
M  END
""",
                "types": ["overlapping_bonds"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)

    def test_check_reaction_queries(self):
        headers, data = self.get_headers(
            {
                "struct": """$RXN



  2  2  0
$MOL

  Ketcher 10051614552D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
   15.8500   -8.0750    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$MOL

  Ketcher 10051614552D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
   18.1250   -8.1750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$MOL

  Ketcher 10051614552D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
   23.0750   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
$MOL

  Ketcher 10051614552D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
   26.1250   -7.9750    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
M  END
""",
                "types": ["query", "valence"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            {
                "valence": "Structure contains query features, so valency could not be checked",
                "query": "Structure contains query features",
            },
            result_data,
        )
        # TODO: Uncomment when Ketcher supports JSON format
        # self.assertEqual({'query': {'reactants': {'0': 'Query'}, 'products': {'1': 'Query'}}, 'valence': {'reactants': {'0': 'Structure contains query features, so valency could not be checked'}, 'products': {'1': 'Structure contains query features, so valency could not be checked'}}}, result_data)

    def test_check_atoms(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 10311615312D 1   1.00000     0.00000     0

  2  0  0     0  0            999 V2000
    0.0000    0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
M  END
"""
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual({}, result_data)

    # TODO: Add validation checks for /check

    def test_json_calculate(self):
        headers, data = self.get_headers(
            {"struct": "C", "properties": ("molecular-weight", "gross")}
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C H4", result_data["gross"])
        self.assertGreater(17, float(result_data["molecular-weight"]))
        self.assertLess(16, float(result_data["molecular-weight"]))

    def test_calculate(self):
        result = requests.post(
            self.url_prefix + "/calculate",
            headers={"Content-Type": "chemical/x-daylight-smiles"},
            data="C",
        )
        self.assertEqual(200, result.status_code)
        result_data = result.text
        self.assertEqual("molecular-weight: 16.0424604", result_data)

    def test_calculate_components_mol(self):
        headers, data = self.get_headers(
            {
                "struct": "C.CC",
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "16.0424604; 30.0690408", result_data["molecular-weight"]
        )
        self.assertEqual("C H4; C2 H6", result_data["gross"])
        self.assertEqual(
            "C 74.87 H 25.13; C 79.89 H 20.11", result_data["mass-composition"]
        )

    def test_calculate_polymer(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 11071615122D 1   1.00000     0.00000     0

  7  6  0     0  0            999 V2000
    6.8750   -6.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.7410   -6.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.6071   -6.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.4731   -6.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3391   -6.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.2051   -6.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0712   -6.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT
M  SMT   1 n
M  SAL   1  1   3
M  SBL   1  2   2   3
M  SDI   1  4    8.1740   -7.1000    8.1740   -5.6000
M  SDI   1  4    9.0401   -5.6000    9.0401   -7.1000
M  STY  1   2 SRU
M  SLB  1   2   2
M  SCN  1   2 HT
M  SMT   2 kk
M  SAL   2  2   4   5
M  SBL   2  2   3   5
M  SDI   2  4    9.0401   -7.1000    9.0401   -5.6000
M  SDI   2  4   10.7721   -5.6000   10.7721   -7.1000
M  END
""",
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "error: Cannot calculate mass for structure with repeating units",
            result_data["molecular-weight"],
        )
        self.assertEqual("C4 H10(C H2)n(C2 H4)kk", result_data["gross"])
        self.assertEqual(
            "error: Cannot calculate mass for structure with repeating units",
            result_data["mass-composition"],
        )

    def test_calculate_components_rxn(self):
        headers, data = self.get_headers(
            {
                "struct": """$RXN



  1  1  0
$MOL

  Ketcher 10271616252D 1   1.00000     0.00000     0

  3  1  0     0  0            999 V2000
    7.8420   -6.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.7080   -6.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5500   -6.3250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
M  END
$MOL

  Ketcher 10271616252D 1   1.00000     0.00000     0

  3  1  0     0  0            999 V2000
   19.9670   -6.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.8330   -5.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   24.9250   -5.6250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
M  END
""",
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "[30.0690408; 16.0424604] > [30.0690408; 16.0424604]",
            result_data["molecular-weight"],
        )
        self.assertEqual("[C2 H6; C H4] > [C2 H6; C H4]", result_data["gross"])
        self.assertEqual(
            "[C 79.89 H 20.11; C 74.87 H 25.13] > [C 79.89 H 20.11; C 74.87 H 25.13]",
            result_data["mass-composition"],
        )

    def test_calculate_rxn(self):
        headers, data = self.get_headers(
            {
                "struct": "C.CC>>CC.C",
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "[16.0424604] + [30.0690408] > [30.0690408] + [16.0424604]",
            result_data["molecular-weight"],
        )
        self.assertEqual(
            "[16.0313001] + [30.0469501] > [30.0469501] + [16.0313001]",
            result_data["most-abundant-mass"],
        )
        self.assertEqual(
            "[16.0313001] + [30.0469501] > [30.0469501] + [16.0313001]",
            result_data["monoisotopic-mass"],
        )
        self.assertEqual(
            "[C H4] + [C2 H6] > [C2 H6] + [C H4]", result_data["gross"]
        )
        self.assertEqual(
            "[C 74.87 H 25.13] + [C 79.89 H 20.11] > [C 79.89 H 20.11] + [C 74.87 H 25.13]",
            result_data["mass-composition"],
        )

    def test_calculate_selected(self):
        headers, data = self.get_headers(
            {
                "struct": "CC",
                "input_format": "chemical/x-mdl-molfile",
                "selected": [
                    0,
                ],
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("15.0345204", result_data["molecular-weight"])
        self.assertEqual("C H3", result_data["gross"])
        self.assertEqual("C 79.89 H 20.11", result_data["mass-composition"])

    def test_calculate_selected_benzene(self):
        headers, data = self.get_headers(
            {
                "struct": "C1(N)=CC=CC=C1",
                "selected": [0, 2, 3, 4, 5, 6],
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C6 H5", result_data["gross"])
        self.assertEqual("77.1039016", result_data["molecular-weight"])
        self.assertEqual("C 93.46 H 6.54", result_data["mass-composition"])

    def test_calculate_empty(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 10211616132D 1   1.00000     0.00000     0

  0  0  0     0  0            999 V2000
M  END
""",
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("", result_data["gross"])
        self.assertEqual("", result_data["molecular-weight"])
        self.assertEqual("", result_data["most-abundant-mass"])
        self.assertEqual("", result_data["monoisotopic-mass"])
        self.assertEqual("", result_data["mass-composition"])

    def test_calculate_selected_benzene_2(self):
        headers, data = self.get_headers(
            {
                "struct": "C1=CC=CC=C1",
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [
                    0,
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C H", result_data["gross"])
        self.assertEqual("13.0186403", result_data["molecular-weight"])
        self.assertEqual("13.007825", result_data["most-abundant-mass"])
        self.assertEqual("13.007825", result_data["monoisotopic-mass"])
        self.assertEqual("C 92.26 H 7.74", result_data["mass-composition"])

    def test_calculate_query_mol(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 11081614202D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    6.4500   -4.5500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.4500   -4.5500    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
    7.9500   -5.4160    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  8  0     0  0
  2  3  1  0     0  0
M  END
""",
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["gross"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["molecular-weight"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["most-abundant-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["monoisotopic-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["mass-composition"],
        )

    def test_calculate_query_mol_selected(self):
        mol = """
  Ketcher 11081614252D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    6.4500   -4.5500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.4500   -4.5500    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
    7.9500   -5.4160    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  8  0     0  0
  2  3  1  0     0  0
M  END
"""
        headers, data = self.get_headers(
            {
                "struct": mol,
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [
                    0,
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["gross"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["molecular-weight"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["most-abundant-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["monoisotopic-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["mass-composition"],
        )
        headers, data = self.get_headers(
            {
                "struct": mol,
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [
                    2,
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C H3", result_data["gross"])
        self.assertEqual("15.0345204", result_data["molecular-weight"])
        self.assertEqual("15.0234751", result_data["most-abundant-mass"])
        self.assertEqual("15.0234751", result_data["monoisotopic-mass"])
        self.assertEqual("C 79.89 H 20.11", result_data["mass-composition"])

    def test_calculate_query_rxn(self):
        headers, data = self.get_headers(
            {
                "struct": """$RXN



  1  1  0
$MOL

  Ketcher 11081614262D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    6.4500   -4.5500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.4500   -4.5500    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
    7.9500   -5.4160    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  8  0     0  0
  2  3  1  0     0  0
M  END
$MOL

  Ketcher 11081614262D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
   13.1000   -4.4920    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.1000   -4.4920    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
   14.6000   -5.3580    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
M  END

""",
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["gross"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["molecular-weight"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["most-abundant-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["monoisotopic-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["mass-composition"],
        )

    def test_calculate_query_rxn_selected(self):
        rxn = """$RXN



  1  1  0
$MOL

  Ketcher 11081614532D 1   1.00000     0.00000     0

  3  2  0     0  0            999 V2000
    0.0000    0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8660   -0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.7321    0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  2  3  1  0     0  0
  1  2  8  0     0  0
M  END
$MOL

  Ketcher 11081614532D 1   1.00000     0.00000     0

  4  3  0     0  0            999 V2000
    7.7321    0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.5980   -0.2500    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    9.4641    0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3301   -0.2500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
M  END
"""
        headers, data = self.get_headers(
            {
                "struct": rxn,
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [
                    0,
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["gross"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["molecular-weight"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["most-abundant-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["monoisotopic-mass"],
        )
        self.assertEqual(
            "Cannot calculate properties for structures with query features",
            result_data["mass-composition"],
        )
        headers, data = self.get_headers(
            {
                "struct": rxn,
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [
                    2,
                ],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C H3", result_data["gross"])
        self.assertEqual("15.0345204", result_data["molecular-weight"])
        self.assertEqual("15.0234751", result_data["most-abundant-mass"])
        self.assertEqual("15.0234751", result_data["monoisotopic-mass"])
        self.assertEqual("C 79.89 H 20.11", result_data["mass-composition"])
        headers, data = self.get_headers(
            {
                "struct": rxn,
                "properties": [
                    "molecular-weight",
                    "most-abundant-mass",
                    "monoisotopic-mass",
                    "gross",
                    "mass-composition",
                ],
                "selected": [2, 3, 4, 5],
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual("C H3; C2 H6 N", result_data["gross"])
        self.assertEqual(
            "15.0345204; 44.0757403", result_data["molecular-weight"]
        )
        self.assertEqual(
            "15.0234751; 44.0500238", result_data["most-abundant-mass"]
        )
        self.assertEqual(
            "15.0234751; 44.0500238", result_data["monoisotopic-mass"]
        )
        self.assertEqual(
            "C 79.89 H 20.11; C 54.50 H 13.72 N 31.78",
            result_data["mass-composition"],
        )

    def test_calculate_selected_components_mol(self):
        headers, data = self.get_headers(
            {
                "struct": "CC.CC",
                "input_format": "chemical/x-mdl-molfile",
                "selected": [0, 2, 3],
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "15.0345204; 30.0690408", result_data["molecular-weight"]
        )
        self.assertEqual("C H3; C2 H6", result_data["gross"])
        self.assertEqual(
            "C 79.89 H 20.11; C 79.89 H 20.11", result_data["mass-composition"]
        )

    def test_calculate_selected_components_rxn(self):
        headers, data = self.get_headers(
            {
                "struct": "CC>>CC.CC",
                "input_format": "chemical/x-mdl-rxnfile",
                "selected": [0, 2, 3],
                "properties": (
                    "molecular-weight",
                    "gross",
                    "mass-composition",
                ),
            }
        )
        result = requests.post(
            self.url_prefix + "/calculate", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertEqual(
            "16.0424604; 30.0690408", result_data["molecular-weight"]
        )
        self.assertEqual("C H4; C2 H6", result_data["gross"])
        self.assertEqual(
            "C 74.87 H 25.13; C 79.89 H 20.11", result_data["mass-composition"]
        )

    def test_convert_inchi_aux(self):
        params = {
            "struct": "c1ccccc1",
            "output_format": "chemical/x-inchi-aux",
        }
        headers, data = self.get_headers(params)
        result = requests.post(
            self.url_prefix + "/convert", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual("chemical/x-inchi-aux", result_data["format"])
        self.assertIn("AuxInfo=", result_data["struct"])

        result = requests.get(self.url_prefix + "/convert", params=params)
        self.assertIn("AuxInfo=", result.text)

    def test_convert_chemaxon_smiles(self):
        params = {
            "struct": "CC[*]",
            "output_format": "chemical/x-chemaxon-cxsmiles",
        }
        headers, data = self.get_headers(params)
        result = requests.post(
            self.url_prefix + "/convert", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual("chemical/x-chemaxon-cxsmiles", result_data["format"])
        self.assertEqual("CC%91.[*]%91", result_data["struct"])

        result = requests.get(self.url_prefix + "/convert", params=params)
        self.assertEqual("CC%91.[*]%91", result.text)

    # TODO: Add validation checks for /calculate

    def test_stereo(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 03071819152D 1   1.00000     0.00000     0

  7  6  0     0  0            999 V2000
    7.2750   -2.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.5338   -3.8409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.4997   -4.0997    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.9997   -4.9658    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.9997   -4.9658    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.6678   -4.3409    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    8.3998   -3.3409    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  2  6  1  0     0  0
  2  7  1  0     0  0
M  END

    """,
                "types": ["query", "stereo"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure contains one or more stereogenic atom(s) with unspecified stereochemistry",
            result_data["stereo"],
        )
        headers, data = self.get_headers(
            {
                "struct": """
Ketcher 03071820162D 1   1.00000     0.00000     0

  7  7  0     0  0            999 V2000
    6.1956   -7.4385    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.5784   -6.6550    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8007   -5.6826    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.6960   -5.2488    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.2021   -7.4385    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.6025   -5.6819    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.8249   -6.6550    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  6  7  1  1     0  0
  5  7  1  1     0  0
  1  5  1  1     0  0
  4  6  1  1     0  0
  3  4  1  1     0  0
  2  3  1  1     0  0
  1  2  1  1     0  0
M  END
""",
                "types": ["query", "stereo"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure has stereochemistry errors", result_data["stereo"]
        )

    def test_chiral(self):
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 12021612452D 1   1.00000     0.00000     0

 45 42  0     0  0            999 V2000
    7.3640   -2.5680    9.2346 Ca  0  0  0  0  0  0  0  0  0  0  0  0
    8.0647   -3.0537   10.1222 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.6967   -2.3175   10.8796 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.3974   -2.8034   11.7673 Nb  0  0  0  0  0  0  0  0  0  0  0  0
   10.0293   -2.0671   12.5246 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.7301   -2.5529   13.4123 Se  0  0  0  0  0  0  0  0  0  0  0  0
   11.3620   -1.8167   14.1698 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0628   -2.3024   15.0573 Ge  0  0  0  0  0  0  0  0  0  0  0  0
   12.6946   -1.5662   15.8148 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.3954   -2.0521   16.7023 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.0273   -1.3158   17.4598 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.7281   -1.8016   18.3475 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.3600   -1.0654   19.1048 S   0  0  0  0  0  0  0  0  0  0  0  0
   16.0608   -1.5511   19.9924 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.6927   -0.8149   20.7499 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.3934   -1.3007   21.6375 C   0  0  0  0  0  0  0  0  0  0  0  0
   18.0254   -0.5645   22.3948 Ca  0  0  0  0  0  0  0  0  0  0  0  0
   17.4624   -2.5227   21.7677 F   0  0  0  0  0  0  0  0  0  0  0  0
   16.1296   -2.7732   20.1226 F   0  0  0  0  0  0  0  0  0  0  0  0
   14.7970   -3.0235   18.4776 F   0  0  0  0  0  0  0  0  0  0  0  0
   13.9584   -0.0938   17.3296 Cl  0  0  0  0  0  0  0  0  0  0  0  0
   12.6258   -0.3443   15.6845 Cl  0  0  0  0  0  0  0  0  0  0  0  0
   11.2932   -0.5947   14.0395 Cl  0  0  0  0  0  0  0  0  0  0  0  0
    9.9604   -0.8451   12.3944 Cl  0  0  0  0  0  0  0  0  0  0  0  0
   15.0153   -6.2242   18.8656 Ca  0  0  0  0  0  0  0  0  0  0  0  0
   15.7847   -6.0797   19.8155 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.7015  -10.4726    8.7015 Se  0  0  0  0  0  0  0  0  0  0  0  0
    7.4024  -10.9583    9.5891 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0343  -10.2221   10.3467 Ge  0  0  0  0  0  0  0  0  0  0  0  0
    8.7351  -10.7079   11.2342 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.3669   -9.9716   11.9915 As  0  0  0  0  0  0  0  0  0  0  0  0
   10.0677  -10.4574   12.8792 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.6997   -9.7213   13.6367 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.4004  -10.2070   14.5242 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0323   -9.4708   15.2817 Ge  0  0  0  0  0  0  0  0  0  0  0  0
   12.7330   -9.9566   16.1693 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.3650   -9.2203   16.9267 Se  0  0  0  0  0  0  0  0  0  0  0  0
   14.0658   -9.7061   17.8144 Se  0  0  0  0  0  0  0  0  0  0  0  0
    9.3423  -10.9133   11.9955 Ge  0  0  0  0  0  0  0  0  0  0  0  0
   10.6307   -8.4993   13.5065 As  0  0  0  0  0  0  0  0  0  0  0  0
   12.9987  -11.0994   16.5408 Se  0  0  0  0  0  0  0  0  0  0  0  0
    7.4713  -12.1804    9.7193 Sb  0  0  0  0  0  0  0  0  0  0  0  0
    7.9654   -9.0001   10.2164 Sb  0  0  0  0  0  0  0  0  0  0  0  0
   11.4693  -11.4291   14.6545 Sb  0  0  0  0  0  0  0  0  0  0  0  0
   10.3334  -11.6004   13.2508 Sb  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  7  8  1  0     0  0
  8  9  1  0     0  0
  9 10  1  0     0  0
 10 11  1  0     0  0
 11 12  1  0     0  0
 12 13  1  0     0  0
 13 14  1  0     0  0
 14 15  1  0     0  0
 15 16  1  0     0  0
 16 17  1  0     0  0
 16 18  1  0     0  0
 14 19  1  0     0  0
 12 20  1  0     0  0
 11 21  1  0     0  0
  9 22  1  0     0  0
  7 23  1  0     0  0
  5 24  1  0     0  0
 25 26  2  0     0  0
 27 28  1  0     0  0
 28 29  1  0     0  0
 29 30  1  0     0  0
 30 31  1  0     0  0
 31 32  1  0     0  0
 32 33  1  0     0  0
 33 34  1  0     0  0
 34 35  1  0     0  0
 35 36  1  0     0  0
 36 37  1  0     0  0
 37 38  1  0     0  0
 32 39  1  0     0  0
 33 40  1  0     0  0
 36 41  1  0     0  0
 28 42  1  0     0  0
 29 43  1  0     0  0
 34 44  1  0     0  0
 32 45  1  0     0  0
M  CHG  2   1  -1  17  -1
M  END
""",
                "types": ["query", "chiral"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure has 3D Chiral center", result_data["chiral"]
        )
        headers, data = self.get_headers(
            {
                "struct": """
  Ketcher 03071819482D 1   1.00000     0.00000     0

  6  6  0     1  1            999 V2000
    5.1750   -4.7000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0410   -5.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0410   -6.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.1750   -6.7000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.3090   -6.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.3090   -5.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  1  1  0     0  0
M  END

""",
                "types": ["query", "chiral"],
            }
        )
        result = requests.post(
            self.url_prefix + "/check", headers=headers, data=data
        )
        result_data = json.loads(result.text)
        self.assertEqual(
            "Structure has invalid Chiral flag", result_data["chiral"]
        )
