"""Tests for layout modifier on the /indigo/convert endpoint.

Covers the `layout` modifier path that applies ``IndigoObject.layout()``
to compounds during conversion. The fix ensures that bond lengths
are consistent when layout is applied.

Note: The HTTP service does not support HELM format directly.
These tests verify layout behaviour through regular molecule input.
"""

from typing import Any, Dict, List, Optional, Tuple

import pytest
from fastapi.testclient import TestClient

from indigo_service.indigo_http import app

client = TestClient(app)


# ── Helpers ──────────────────────────────────────────


def convert_request(
    structure: str,
    output_format: str = "molfile",
    modifiers: Optional[List[str]] = None,
    input_format: str = "auto",
) -> Dict[str, Any]:
    """Build a JSON:API convert request payload."""
    compound: Dict[str, Any] = {
        "structure": structure,
        "format": input_format,
    }
    if modifiers is not None:
        compound["modifiers"] = modifiers
    else:
        compound["modifiers"] = []
    return {
        "data": {
            "type": "convert",
            "attributes": {
                "compound": compound,
                "outputFormat": output_format,
            },
        }
    }


def molfile_atom_coords(molfile: str) -> List[Tuple[float, float]]:
    """Extract 2D atom coordinates from a V2000 or V3000 molfile string.

    Parses only V2000 atom block (lines after counts line, before bond block).
    """
    coords: List[Tuple[float, float]] = []
    lines = molfile.strip().splitlines()
    # V2000: line 4 is the counts line (atom_count bond_count ...)
    if len(lines) < 5:
        return coords
    counts_line = lines[3].strip().split()
    try:
        n_atoms = int(counts_line[0])
    except (ValueError, IndexError):
        return coords
    for i in range(4, min(4 + n_atoms, len(lines))):
        parts = lines[i].strip().split()
        if len(parts) >= 3:
            coords.append((float(parts[0]), float(parts[1])))
    return coords


def bond_lengths_from_coords(
    coords: List[Tuple[float, float]], molfile: str
) -> List[float]:
    """Compute bond lengths from a molfile's bond block."""
    import math

    lengths = []
    lines = molfile.strip().splitlines()
    counts_line = lines[3].strip().split()
    n_atoms = int(counts_line[0])
    n_bonds = int(counts_line[1])
    bond_start = 4 + n_atoms
    for i in range(bond_start, min(bond_start + n_bonds, len(lines))):
        parts = lines[i].strip().split()
        if len(parts) >= 2:
            a1 = int(parts[0]) - 1  # 1-based → 0-based
            a2 = int(parts[1]) - 1
            if 0 <= a1 < len(coords) and 0 <= a2 < len(coords):
                dx = coords[a2][0] - coords[a1][0]
                dy = coords[a2][1] - coords[a1][1]
                lengths.append(math.sqrt(dx * dx + dy * dy))
    return lengths


# ==========================================================================
#  Layout modifier — basic functionality
# ==========================================================================


class TestLayoutModifier:
    """The 'layout' modifier on /indigo/convert should produce valid 2D coords."""

    def test_layout_produces_coordinates(self) -> None:
        """Converting with layout modifier should produce non-zero coordinates."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC=CC=C1",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        assert len(coords) >= 6, "Benzene should have >= 6 atoms"

        # At least some coords should be non-zero
        non_zero = [
            c for c in coords if abs(c[0]) > 0.001 or abs(c[1]) > 0.001
        ]
        assert (
            len(non_zero) > 0
        ), "All coordinates are zero — layout not applied"

    def test_layout_bond_lengths_uniform(self) -> None:
        """After layout, all bonds in benzene should have similar length."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC=CC=C1",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        lengths = bond_lengths_from_coords(coords, molfile)

        assert len(lengths) >= 6
        avg = sum(lengths) / len(lengths)
        for i, length in enumerate(lengths):
            assert length == pytest.approx(
                avg, abs=0.2
            ), f"Bond {i} length {length:.3f} != avg {avg:.3f}"

    def test_layout_with_aromatize(self) -> None:
        """Layout + aromatize modifiers should both be applied."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC=CC=C1",
                output_format="smiles",
                modifiers=["aromatize", "layout"],
            ),
        )
        assert response.status_code == 200
        result = response.json()["data"]["attributes"]["structure"]
        assert result == "c1ccccc1", f"Expected aromatic SMILES, got: {result}"

    def test_layout_modifier_alone(self) -> None:
        """Layout alone without other modifiers should work."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="CCCCCC",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        assert len(coords) >= 6

    def test_no_layout_modifier(self) -> None:
        """Without layout modifier, conversion should still succeed."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="CNC",
                output_format="smiles",
                modifiers=[],
            ),
        )
        assert response.status_code == 200
        result = response.json()["data"]["attributes"]["structure"]
        assert result == "CNC"


# ==========================================================================
#  Layout modifier — complex molecules
# ==========================================================================


class TestLayoutComplex:
    """Layout on molecules with rings, chains, and mixed topologies."""

    def test_chain_molecule_layout(self) -> None:
        """Linear alkane should get valid layout coordinates."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="CCCCCCCCCC",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        lengths = bond_lengths_from_coords(coords, molfile)

        assert len(lengths) >= 9
        for i, length in enumerate(lengths):
            assert length > 0.5, f"Chain bond {i} is too short: {length:.3f}"

    def test_fused_rings_layout(self) -> None:
        """Naphthalene (fused rings) should produce valid layout."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC2=CC=CC=C2C=C1",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        assert len(coords) >= 10

        lengths = bond_lengths_from_coords(coords, molfile)
        assert len(lengths) >= 11
        avg = sum(lengths) / len(lengths)
        for i, length in enumerate(lengths):
            assert length == pytest.approx(
                avg, abs=0.3
            ), f"Fused ring bond {i}: {length:.3f} vs avg {avg:.3f}"

    def test_layout_idempotent_via_molfile(self) -> None:
        """Applying layout twice via re-conversion should give consistent coords."""
        # First layout
        resp1 = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC=CC=C1",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert resp1.status_code == 200
        molfile1 = resp1.json()["data"]["attributes"]["structure"]

        # Second layout on the output of the first
        resp2 = client.post(
            "/indigo/convert",
            json=convert_request(
                structure=molfile1,
                output_format="molfile",
                input_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert resp2.status_code == 200
        molfile2 = resp2.json()["data"]["attributes"]["structure"]

        coords1 = molfile_atom_coords(molfile1)
        coords2 = molfile_atom_coords(molfile2)
        assert len(coords1) == len(coords2)

        lengths1 = bond_lengths_from_coords(coords1, molfile1)
        lengths2 = bond_lengths_from_coords(coords2, molfile2)
        assert len(lengths1) == len(lengths2)
        for i, (l1, l2) in enumerate(zip(lengths1, lengths2)):
            assert l1 == pytest.approx(
                l2, abs=0.15
            ), f"Bond {i} changed: {l1:.3f} → {l2:.3f}"


# ==========================================================================
#  Error handling
# ==========================================================================


class TestLayoutErrors:
    """Error scenarios for layout via convert endpoint."""

    def test_invalid_structure_returns_error(self) -> None:
        """Invalid SMILES with layout modifier should return error."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="INVALID_NOT_A_MOLECULE",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        # Service returns 400 for unparseable structures
        assert response.status_code in (400, 422)

    def test_empty_structure_returns_error(self) -> None:
        """Empty string input should return error."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="",
                output_format="molfile",
                modifiers=["layout"],
            ),
        )
        assert response.status_code in (400, 422)


# ==========================================================================
#  Clean2d modifier (related to layout)
# ==========================================================================


class TestClean2dModifier:
    """The 'clean2d' modifier should also produce valid coordinates."""

    def test_clean2d_produces_coordinates(self) -> None:
        """Converting with clean2d should produce valid molfile."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="C1=CC=CC=C1",
                output_format="molfile",
                modifiers=["clean2d"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        assert len(coords) >= 6

    def test_clean2d_bond_lengths_reasonable(self) -> None:
        """Clean2d should produce reasonable bond lengths."""
        response = client.post(
            "/indigo/convert",
            json=convert_request(
                structure="CCCCCC",
                output_format="molfile",
                modifiers=["clean2d"],
            ),
        )
        assert response.status_code == 200
        molfile = response.json()["data"]["attributes"]["structure"]
        coords = molfile_atom_coords(molfile)
        lengths = bond_lengths_from_coords(coords, molfile)

        for i, length in enumerate(lengths):
            assert (
                length > 0.3
            ), f"Bond {i} near-zero after clean2d: {length:.3f}"
