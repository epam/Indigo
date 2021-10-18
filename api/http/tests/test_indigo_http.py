import itertools

from fastapi.testclient import TestClient

from indigo_service import jsonapi
from indigo_service.indigo_http import app

client = TestClient(app)

test_structures = [
    {"structure": "CNC", "format": "auto"},
    {"structure": "CN1C=NC2=C1C(=O)N(C(=O)N2C)C", "format": "auto"},
]


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


def descriptors_request(compound: dict, descriptors: list) -> dict:
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
