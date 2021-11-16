import itertools
import os
import pathlib
from typing import List

import pytest

from fastapi.testclient import TestClient

from indigo_service import jsonapi
from indigo_service.indigo_http import app

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
    resources = "tests/test_resources/kets"
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
    source: dict,
    targets: list[dict],
    fingerprint: str = "sim",
    metric: str = "tanimoto",
    alpha: float = 0.5,
    beta: float = 0.5,
) -> dict:
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


def test_similarities_error():
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


def descriptors_request(compound: dict, descriptors: tuple) -> dict:
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
