#!/usr/bin/env python3
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from common.util import compare_diff  # noqa

from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)


def test_helm_case_insensitive():
    """Test that HELM aliases work case-insensitively."""
    indigo = Indigo()
    indigo.setOption("json-saving-pretty", True)

    ref_path = joinPathPy("ref/", __file__)
    lib = indigo.loadMonomerLibraryFromFile(
        os.path.join(ref_path, "monomer_library.ket")
    )

    test_cases = [
        # (HELM string, description)
        ("PEPTIDE1{cya}$$$$V2.0", "helm_cysteic_acid_1"),
        ("PEPTIDE1{CYA}$$$$V2.0", "helm_cysteic_acid_2"),
        ("PEPTIDE1{Cya}$$$$V2.0", "helm_cysteic_acid_3"),
        ("PEPTIDE1{cYa}$$$$V2.0", "helm_cysteic_acid_4"),
        ("PEPTIDE1{CyA}$$$$V2.0", "helm_cysteic_acid_5"),
        # Test with multiple residues
        (
            "PEPTIDE1{cya.cya.cya.meg.iva}$$$$V2.0",
            "helm_peptide_chain_1",
        ),
        (
            "PEPTIDE1{CYA.CYA.CYA.MEG.IVA}$$$$V2.0",
            "helm_peptide_chain_2",
        ),
        (
            "PEPTIDE1{Cya.cya.CYA.Meg.Iva}$$$$V2.0",
            "helm_peptide_chain_3",
        ),
        # Test RNA bases
        ("RNA1{R(A)P}$$$$V2.0", "helm_rna_base_a_1"),
        ("RNA1{R(a)P}$$$$V2.0", "helm_rna_base_a_2"),
        # Test sugars
        ("RNA1{r}$$$$V2.0", "helm_sugar_r_1"),
        ("RNA1{R}$$$$V2.0", "helm_sugar_r_2"),
    ]

    print("Testing HELM case-insensitive alias support...")
    print("=" * 60)

    for helm_string, filename in test_cases:
        try:
            mol = indigo.loadHelm(helm_string, lib)
            ket = mol.json()
            compare_diff(ref_path, filename + ".ket", ket)
        except IndigoException as e:
            print(f"✗ FAIL: {helm_string}")
            print(f"  Error: {getIndigoExceptionText(e)}")
        print()

    return


if __name__ == "__main__":
    test_helm_case_insensitive()
