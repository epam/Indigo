import base64
import io
import itertools
import os
import pathlib
import xml.etree.ElementTree as elTree
from typing import Any, BinaryIO, Dict, List, Optional

import PyPDF2
import pytest
from fastapi.testclient import TestClient
from PIL import Image
from requests import Response

from indigo_service import jsonapi
from indigo_service.indigo_http import app
from indigo_service.jsonapi import Descriptors

client = TestClient(app)

test_structures = [
    {"structure": "CNC", "format": "auto"},
    {"structure": "CN1C=NC2=C1C(=O)N(C(=O)N2C)C", "format": "auto"},
    {
        "structure": "InChI=1S/C8H10N4O2/c1-10-4-9-6-5(10)"
        "7(13)12(3)8(14)11(6)2/h4H,1-3H3",
        "format": "auto",
    },
]


# Convert


@pytest.mark.parametrize(
    "test_input,target,modifiers,expected",
    [
        (["CNC", "inchi", [], "InChI=1S/C2H7N/c1-3-2/h3H,1-2H3"]),
        (["InChI=1S/C2H7N/c1-3-2/h3H,1-2H3", "smiles", [], "CNC"]),
        (
            [
                "InChI=1S/C8Cl2N2O2/c9-5-6(10)8(14)4(2-12)3(1-11)7(5)13",
                "inchi",
                ["aromatize", "clean2d"],
                "InChI=1S/C8Cl2N2O2/c9-5-6(10)8(14)4(2-12)3(1-11)7(5)13",
            ]
        ),
        (["C1=CC=CC=C1", "smiles", ["aromatize"], "c1ccccc1"]),
    ],
)
def test_convert(
    test_input: str, target: str, modifiers: List[str], expected: str
) -> None:
    response = client.post(
        "/indigo/convert",
        json={
            "data": {
                "type": "convert",
                "attributes": {
                    "compound": {
                        "structure": test_input,
                        "format": "auto",
                        "modifiers": modifiers,
                    },
                    "outputFormat": target,
                },
            }
        },
    )
    assert response.status_code == 200
    assert response.json()["data"]["attributes"]["structure"] == expected
    assert response.json()["data"]["attributes"]["format"] == target


def test_ket_convert() -> None:
    resources = pathlib.Path(__file__).parent / "test_resources/kets"
    *_, files = next(os.walk(resources))
    for file_ in files:
        # pylint: disable=unspecified-encoding
        with open(pathlib.Path(resources) / file_):
            smiles = pathlib.Path(file_).stem
            # ket = json.loads(f.read())
            response = client.post(
                "/indigo/convert",
                json={
                    "data": {
                        "type": "convert",
                        "attributes": {
                            "compound": {
                                "structure": smiles,
                                "format": "auto",
                                "modifiers": [],
                            },
                            "outputFormat": "ket",
                        },
                    }
                },
            )
            assert response.status_code == 200


# Similarities


def similarity_request(  # pylint: disable=too-many-arguments
    source: dict[Any, Any],
    targets: list[dict[Any, Any]],
    fingerprint: str = "sim",
    metric: str = "tanimoto",
    alpha: float = 0.5,
    beta: float = 0.5,
) -> dict[str, Any]:
    return {
        "data": {
            "type": "similarities",
            "attributes": {
                "source": source,
                "targets": targets,
                "fingerprint": fingerprint,
                "metric": metric,
                "alpha": alpha,
                "beta": beta,
            },
        }
    }


def test_similarities_error() -> None:
    for structure in test_structures:
        response = client.post(
            "/indigo/similarities",
            json=similarity_request(
                source=structure,
                targets=[{"structure": "D", "format": "auto"}],
            ),
        )
        assert isinstance(response.json().get("errors"), list)
        assert response.status_code == 400
        assert len(response.json().get("errors")) == 1


# Descriptors


def descriptors_request(
    compound: dict[Any, Any], descriptors: tuple[Descriptors, ...]
) -> dict[str, Any]:
    return {
        "data": {
            "type": "descriptor",
            "attributes": {"compound": compound, "descriptors": descriptors},
        }
    }


react_descriptors = [
    jsonapi.Descriptors.COUNT_CATALYSTS,
    jsonapi.Descriptors.COUNT_MOLECULES,
    jsonapi.Descriptors.COUNT_PRODUCTS,
]
mol_descriptors = list(
    filter(lambda x: x not in react_descriptors, jsonapi.Descriptors)
)


def test_base_descriptors() -> None:
    max_iters = 10

    for perm_number, descriptors in enumerate(
        itertools.permutations(mol_descriptors, 4)
    ):
        if perm_number == max_iters:
            break
        for compound in test_structures:
            response = client.post(
                "/indigo/descriptors",
                json=descriptors_request(
                    compound=compound, descriptors=descriptors
                ),
            )
            assert response.status_code == 200


# Render


def render_request(
    structure: str,
    output_format: str,
    options: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    return {
        "data": {
            "type": "render",
            "attributes": {
                "compound": {"structure": structure, "format": "auto"},
                "outputFormat": output_format,
                "options": options,
            },
        }
    }


def decode_image(response: Response, output_format: str) -> BinaryIO:
    base64_image = response.json()["data"]["attributes"]["image"]
    str_image = base64_image.replace(f"data:{output_format};base64,", "")
    decoded_image = base64.b64decode(str_image)
    return io.BytesIO(decoded_image)


correct_options = {
    "render-coloring": 1,
    "render-bond-line-width": 1.5,
    "render-background-color": "255, 179, 179",
    "render-comment": "COMMENT",
    "render-comment-alignment": "center",
    "render-image-height": 400,
    "render-image-width": 500,
}

incorrect_options = {
    "render-atom-ids-visible": 11,
    "render-highlight-color": "whatever",
    "render-bond-length": "true",
    "render-catalysts-placement": "set something",
}


def test_render_png_base64() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(structure="C", output_format="image/png"),
    )
    png_image = Image.open(decode_image(response, "image/png"))
    assert response.status_code == 200
    assert png_image.format == "PNG"


def test_render_svg_base64() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(structure="C", output_format="image/svg+xml"),
    )
    svg_image = elTree.parse(decode_image(response, "image/svg+xml"))
    assert response.status_code == 200
    assert svg_image.getroot().tag == "{http://www.w3.org/2000/svg}svg"


def test_render_pdf_base64() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(structure="C", output_format="application/pdf"),
    )
    read_file = PyPDF2.PdfFileReader(decode_image(response, "application/pdf"))
    pages_number = read_file.numPages
    assert response.status_code == 200
    assert pages_number == 1


def test_render_correct_png_base64_options() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(
            structure="C",
            output_format="image/png",
            options=correct_options,
        ),
    )
    png_image = Image.open(decode_image(response, "image/png"))
    assert response.status_code == 200
    assert png_image.size == (500, 400)


def test_render_correct_svg_options() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(
            structure="C",
            output_format="image/svg+xml",
            options=correct_options,
        ),
    )
    svg_image = elTree.parse(decode_image(response, "image/svg+xml"))
    width = svg_image.getroot().attrib.get("width")
    height = svg_image.getroot().attrib.get("height")
    assert response.status_code == 200
    assert width == "500"
    assert height == "400"


def test_render_pdf_base64_correct_options() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(
            structure="C",
            output_format="application/pdf",
            options=correct_options,
        ),
    )
    read_file = PyPDF2.PdfFileReader(decode_image(response, "application/pdf"))
    page_height = read_file.getPage(0).mediaBox.getHeight()
    page_width = read_file.getPage(0).mediaBox.getWidth()
    assert response.status_code == 200
    assert page_height == 400
    assert page_width == 500


def test_render_incorrect_format() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(structure="C", output_format="bla"),
    )
    assert response.status_code == 400
    assert response.json()["errors"][0]["detail"] == "bad option"


def test_render_two_output_formats() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(
            structure="C",
            output_format="image/png",
            options={"render-output-format": "image/svg+xml"},
        ),
    )
    error_msg = response.json()["detail"]
    assert response.status_code == 400
    assert error_msg == "Choose only one output format"


def test_render_incorrect_options() -> None:
    response = client.post(
        "/indigo/render",
        json=render_request(
            structure="C",
            output_format="image/svg+xml",
            options=incorrect_options,
        ),
    )
    error_msg = response.json()["errors"][0]["detail"]
    assert response.status_code == 400
    assert error_msg == (
        'option manager: Cannot recognize "whatever" as a color value'
    )


# TODO: /indigo/render with alternative responses types
# def render_request(
#     structure: str, output_format: str, options: Dict = None
# ) -> Dict:
#     return {
#         "data": {
#             "type": "render",
#             "attributes": {
#                 "compound": {"structure": structure, "format": "auto"},
#                 "outputFormat": output_format,
#                 "options": options,
#             },
#         }
#     }
#
#
# correct_options = {
#     "render-coloring": 1,
#     "render-bond-line-width": 1.5,
#     "render-background-color": "255, 179, 179",
#     "render-comment": "COMMENT",
#     "render-comment-alignment": "center",
#     "render-image-height": 400,
#     "render-image-width": 500
# }
#
# incorrect_options = {
#     "render-atom-ids-visible": 11,
#     "render-highlight-color": "doesn't matter",
#     "render-bond-length": "true",
#     "render-catalysts-placement": "set something",
# }
#
#
# def test_render_png() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C", output_format="image/png"
#         )
#     )
#     image = Image.open(io.BytesIO(response.content))
#     assert response.status_code == 200
#     assert response.headers["Content-Type"] == "image/png"
#     assert image.format == "PNG"
#
#
# def test_render_svg() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C", output_format="image/svg+xml"
#         )
#     )
#     image = elTree.fromstring(response.content)
#     assert response.status_code == 200
#     assert response.headers["Content-Type"] == "image/svg+xml"
#     assert image.tag == "{http://www.w3.org/2000/svg}svg"
#
#
# def test_render_png_base64() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C", output_format="image/png;base64"
#         )
#     )
#     base64_image = response.content.replace(b"data:image/png;base64,", b"")
#     decoded_image = base64.b64decode(base64_image)
#     png_image = Image.open(io.BytesIO(decoded_image))
#     assert response.status_code == 200
#     assert response.headers["Content-Type"] == "image/png"
#     assert png_image.format == "PNG"
#
#
# def test_render_pdf() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C", output_format="application/pdf"
#         )
#     )
#     read_file = PyPDF2.PdfFileReader(io.BytesIO(response.content))
#     pages_number = read_file.numPages
#     assert response.status_code == 200
#     assert response.headers["Content-Type"] == "application/pdf"
#     assert (
#         response.headers["Content-Disposition"]
#         == "attachment; filename=mol.pdf"
#     )
#     assert pages_number == 1
#
#
# def test_render_correct_png_options() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="image/png",
#             options=correct_options
#         )
#     )
#     image = Image.open(io.BytesIO(response.content))
#     assert response.status_code == 200
#     assert image.size == (500, 400)
#
#
# def test_render_correct_svg_options() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="image/svg+xml",
#             options=correct_options,
#         )
#     )
#     image = elTree.fromstring(response.content)
#     width = image.attrib.get("width")
#     height = image.attrib.get("height")
#     assert response.status_code == 200
#     assert width == "500"
#     assert height == "400"
#
#
# def test_render_correct_png_base64_options() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="image/png;base64",
#             options=correct_options,
#         )
#     )
#     base64_image = response.content.replace(b"data:image/png;base64,", b"")
#     decoded_image = base64.b64decode(base64_image)
#     png_image = Image.open(io.BytesIO(decoded_image))
#     assert response.status_code == 200
#     assert png_image.size == (500, 400)
#
#
# def test_render_correct_pdf_options() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="application/pdf",
#             options=correct_options,
#         )
#     )
#     read_file = PyPDF2.PdfFileReader(io.BytesIO(response.content))
#     page_height = read_file.getPage(0).mediaBox.getHeight()
#     page_width = read_file.getPage(0).mediaBox.getWidth()
#     assert response.status_code == 200
#     assert page_height == 400
#     assert page_width == 500
#
#
# def test_render_incorrect_format() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(structure="C", output_format="bla")
#     )
#     assert response.status_code == 400
#
#
# def test_render_two_output_formats() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="image/png",
#             options={"render-output-format": "image/svg+xml"}
#         )
#     )
#     assert response.status_code == 400
#
#
# def test_render_incorrect_options() -> None:
#     response = client.post(
#         "/indigo/render",
#         json=render_request(
#             structure="C",
#             output_format="image/svg+xml",
#             options=incorrect_options,
#         )
#     )
#     assert response.status_code == 400
