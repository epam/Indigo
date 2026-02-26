"""Tests for sequence layout bond length consistency.

Covers the fix for Bug:
  When laying out a HELM peptide with a ring (via 'Arrange as Ring'),
  the tail chains would get shorter bond lengths than the ring itself.
  After the fix, `_calculatePos` uses `DEFAULT_MONOMER_BOND_LENGTH` (1.5)
  for tail chains when `sequence_layout && _n_fixed > 0`.
"""

import math

import pytest

from indigo import Indigo, IndigoException

# ── Constants (mirror metalayout.h) ──────────────────
MONOMER_BOND_LENGTH = 1.5
TOLERANCE = 0.25  # absolute tolerance for bond length comparisons


# ── Helpers ──────────────────────────────────────────
def _bond_lengths(mol):
    """Return list of all bond lengths (2D Euclidean)."""
    lengths = []
    for bond in mol.iterateBonds():
        src = bond.source()
        dst = bond.destination()
        sx, sy, _ = src.xyz()
        dx, dy, _ = dst.xyz()
        lengths.append(math.hypot(dx - sx, dy - sy))
    return lengths


def _select_atoms(mol, indices):
    """Mark atoms at given indices as selected."""
    for idx in indices:
        mol.getAtom(idx).select()


# ── Fixtures ─────────────────────────────────────────
@pytest.fixture(scope="module")
def indigo():
    """Shared Indigo session for all tests in this module."""
    return Indigo()


@pytest.fixture(scope="module")
def library(indigo):
    """Minimal monomer library (empty — built-in monomers suffice for peptides)."""
    return indigo.loadMonomerLibrary('{"root":{}}')


# ==========================================================================
#  Selection API tests — validates new select/unselect/isSelected bindings
# ==========================================================================
class TestSelectionApi:
    """Unit tests for the new select()/unselect()/isSelected() Python API."""

    def test_select_marks_atom(self, indigo):
        """select() should mark atom as selected."""
        mol = indigo.loadMolecule("CNC")
        atom = mol.getAtom(0)
        assert not atom.isSelected()
        atom.select()
        assert atom.isSelected()

    def test_unselect_clears_atom(self, indigo):
        """unselect() should clear selection."""
        mol = indigo.loadMolecule("CNC")
        atom = mol.getAtom(1)
        atom.select()
        assert atom.isSelected()
        atom.unselect()
        assert not atom.isSelected()

    def test_has_selection_on_molecule(self, indigo):
        """hasSelection() should reflect atom-level selection state."""
        mol = indigo.loadMolecule("CNC")
        assert not mol.hasSelection()
        mol.getAtom(0).select()
        assert mol.hasSelection()

    def test_select_bond(self, indigo):
        """select() should work on bonds too."""
        mol = indigo.loadMolecule("CNC")
        bond = mol.getBond(0)
        assert not bond.isSelected()
        bond.select()
        assert bond.isSelected()


# ==========================================================================
#  Linear chain tests
# ==========================================================================
class TestLinearChainLayout:
    """Bond lengths on HELM chains without any cycle."""

    def test_uniform_bond_length(self, indigo, library):
        """All inter-monomer bonds in a linear peptide must have equal length."""
        mol = indigo.loadHelm("PEPTIDE1{C.C.C.C.C}$$$$V2.0", library)
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0, "No bonds found"

        for i, length in enumerate(lengths):
            assert length == pytest.approx(
                MONOMER_BOND_LENGTH, abs=TOLERANCE
            ), f"Bond {i} length {length:.3f} != expected {MONOMER_BOND_LENGTH}"

    def test_single_monomer_no_crash(self, indigo, library):
        """Layout of a single monomer should not crash or raise."""
        mol = indigo.loadHelm("PEPTIDE1{C}$$$$V2.0", library)
        mol.layout()
        # Single monomer has no bonds — just verify no exception
        assert mol.countAtoms() >= 1

    def test_two_monomers(self, indigo, library):
        """Minimal chain: two monomers, one bond."""
        mol = indigo.loadHelm("PEPTIDE1{C.C}$$$$V2.0", library)
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) >= 1
        assert lengths[0] == pytest.approx(
            MONOMER_BOND_LENGTH, abs=TOLERANCE
        )


# ==========================================================================
#  Ring layout tests
# ==========================================================================
class TestRingLayout:
    """Bond lengths when monomers form a macrocyclic ring."""

    def test_pure_ring_uniform(self, indigo, library):
        """A cyclic hexapeptide ring should have uniform bond lengths."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,1:R3-6:R3$$$V2.0",
            library,
        )
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0, "No bonds found in pure ring"

        min_len = min(lengths)
        max_len = max(lengths)
        assert (max_len - min_len) < TOLERANCE * 2, (
            f"Ring bond variance too high: "
            f"min={min_len:.3f} max={max_len:.3f}"
        )

    def test_minimal_ring_triangle(self, indigo, library):
        """3-monomer ring: smallest possible cycle."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C}$PEPTIDE1,PEPTIDE1,1:R3-3:R3$$$V2.0",
            library,
        )
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) >= 3
        for i, length in enumerate(lengths):
            assert length > 0.3, f"Bond {i} collapsed to {length:.3f}"

    def test_large_ring_10_monomers(self, indigo, library):
        """10-monomer ring: verifies regular polygon layout."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C.C.C}"
            "$PEPTIDE1,PEPTIDE1,1:R3-10:R3$$$V2.0",
            library,
        )
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0, "No bonds found in large ring"

        avg = sum(lengths) / len(lengths)
        for i, length in enumerate(lengths):
            assert length == pytest.approx(avg, abs=TOLERANCE), (
                f"Bond {i} = {length:.3f}, expected ~{avg:.3f}"
            )


# ==========================================================================
#  Ring with tail — the Bug_1 regression scenario
# ==========================================================================
class TestRingWithTailLayout:
    """
    The core regression: ring + chain tail must have consistent bond lengths
    when 'Arrange as Ring' is applied to a selection.
    """

    def test_exact_arrange_as_ring_helm(self, indigo, library):
        """
        Exact HELM from Arrange as Ring bug report:
        PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,5:R3-8:R3$$$V2.0

        After selecting ring + right tail and calling layout, tail bonds
        must NOT be shorter than ring bonds.
        """
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C}"
            "$PEPTIDE1,PEPTIDE1,5:R3-8:R3$$$V2.0",
            library,
        )

        n_atoms = mol.countAtoms()
        # Select ring + right tail (atoms 4..12, 0-based)
        _select_atoms(mol, range(4, n_atoms))
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0

        min_len = min(lengths)
        max_len = max(lengths)

        # Before the fix: min ≈ 1.0 (tail), max ≈ 1.5 (ring).
        # After fix: all should be close to 1.5.
        assert (max_len - min_len) < TOLERANCE * 2, (
            f"Tail bonds still shorter than ring bonds: "
            f"min={min_len:.3f} max={max_len:.3f}"
        )

        for i, length in enumerate(lengths):
            assert length == pytest.approx(
                MONOMER_BOND_LENGTH, abs=TOLERANCE
            ), f"Bond {i} = {length:.3f}, expected ~{MONOMER_BOND_LENGTH}"

    def test_ring_with_left_tail(self, indigo, library):
        """Ring at the end of the chain — left tail should also be consistent."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C.C.C}"
            "$PEPTIDE1,PEPTIDE1,7:R3-10:R3$$$V2.0",
            library,
        )

        # Select ring + left tail (all atoms)
        mol.layout()

        lengths = _bond_lengths(mol)
        min_len = min(lengths)
        max_len = max(lengths)

        assert (max_len - min_len) < TOLERANCE * 2, (
            f"Left tail inconsistency: min={min_len:.3f} max={max_len:.3f}"
        )

    def test_ring_with_both_tails(self, indigo, library):
        """Ring in the middle with tails on both sides."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C}"
            "$PEPTIDE1,PEPTIDE1,8:R3-13:R3$$$V2.0",
            library,
        )

        # Select ring + right tail (atoms 7..19)
        _select_atoms(mol, range(7, 20))
        mol.layout()

        lengths = _bond_lengths(mol)

        for i, length in enumerate(lengths):
            assert length > MONOMER_BOND_LENGTH * 0.6, (
                f"Bond {i} collapsed to {length:.3f} "
                f"(min acceptable={MONOMER_BOND_LENGTH * 0.6:.3f})"
            )


# ==========================================================================
#  Selection-based layout without ring
# ==========================================================================
class TestSelectionLayoutNoRing:
    """
    Tests _calculatePos path with sequence_layout && _n_fixed > 0
    on pure chains (no ring component).
    """

    def test_partial_selection_preserves_length(self, indigo, library):
        """Selecting part of a linear chain and calling layout."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C}$$$$V2.0", library
        )

        _select_atoms(mol, [4, 5, 6, 7])
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0

        for i, length in enumerate(lengths):
            assert length > MONOMER_BOND_LENGTH * 0.5, (
                f"Bond {i} too short after selected layout: {length:.3f}"
            )

    def test_full_selection_equals_no_selection(self, indigo, library):
        """Selecting ALL atoms should produce the same result as no selection."""
        helm = "PEPTIDE1{C.C.C.C.C}$$$$V2.0"

        # Layout without selection
        mol1 = indigo.loadHelm(helm, library)
        mol1.layout()
        lengths1 = _bond_lengths(mol1)

        # Layout with full selection
        mol2 = indigo.loadHelm(helm, library)
        _select_atoms(mol2, range(mol2.countAtoms()))
        mol2.layout()
        lengths2 = _bond_lengths(mol2)

        assert len(lengths1) == len(lengths2)
        for i, (l1, l2) in enumerate(zip(lengths1, lengths2)):
            assert l1 == pytest.approx(l2, abs=TOLERANCE), (
                f"Bond {i}: no-sel={l1:.3f} vs all-sel={l2:.3f}"
            )


# ==========================================================================
#  Multiple rings
# ==========================================================================
class TestMultipleRings:
    """Several disjoint rings in one peptide chain."""

    def test_two_rings_consistent(self, indigo, library):
        """Two separate macrocycles in a single HELM chain."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C.C.C}"
            "$PEPTIDE1,PEPTIDE1,2:R3-5:R3|"
            "PEPTIDE1,PEPTIDE1,10:R3-13:R3$$$V2.0",
            library,
        )
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0

        min_len = min(lengths)
        max_len = max(lengths)

        assert (max_len - min_len) < TOLERANCE * 3, (
            f"Multi-ring inconsistency: min={min_len:.3f} max={max_len:.3f}"
        )


# ==========================================================================
#  Edge cases and robustness
# ==========================================================================
class TestEdgeCases:
    """Boundary and stress scenarios."""

    def test_empty_library_with_simple_helm(self, indigo):
        """Empty library should still load simple peptides."""
        lib = indigo.loadMonomerLibrary('{"root":{}}')
        mol = indigo.loadHelm("PEPTIDE1{C.C.C}$$$$V2.0", lib)
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) >= 2

    def test_layout_idempotent(self, indigo, library):
        """Calling layout twice should produce the same coordinates."""
        mol = indigo.loadHelm(
            "PEPTIDE1{C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,2:R3-5:R3$$$V2.0",
            library,
        )

        mol.layout()
        lengths1 = _bond_lengths(mol)

        mol.layout()
        lengths2 = _bond_lengths(mol)

        assert len(lengths1) == len(lengths2)
        for i, (l1, l2) in enumerate(zip(lengths1, lengths2)):
            assert l1 == pytest.approx(l2, abs=0.1), (
                f"Bond {i} changed between layouts: {l1:.3f} → {l2:.3f}"
            )

    def test_long_chain_30_monomers(self, indigo, library):
        """Stress: 30-monomer chain should not crash or produce zero-length bonds."""
        monomers = ".".join(["C"] * 30)
        mol = indigo.loadHelm(
            f"PEPTIDE1{{{monomers}}}$$$$V2.0", library
        )
        mol.layout()

        lengths = _bond_lengths(mol)
        assert len(lengths) > 0, "No bonds found in 30-monomer chain"

        for i, length in enumerate(lengths):
            assert length > 0.1, f"Bond {i} is near-zero: {length:.3f}"
