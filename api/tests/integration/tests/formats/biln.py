import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import find_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** BILN interop ***")

ref = joinPathPy("ref/", __file__)
lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

biln_to_helm = {
    "biln_two_backbones": (
        "A-C-D.E-F-G",
        "PEPTIDE1{A.C.D}|PEPTIDE2{E.F.G}$$$$V2.0",
    ),
    "biln_underscore_alias": (
        "A-1Nal-Cys_Bn-C",
        "PEPTIDE1{A.[1Nal].[Cys_Bn].C}$$$$V2.0",
    ),
    "biln_bracketed_alias": (
        "A-[D-1Nal]-[Cys_Bn]-[C]",
        "PEPTIDE1{A.[D-1Nal].[Cys_Bn].C}$$$$V2.0",
    ),
    "biln_star_alias": (
        "A-D*-C",
        "PEPTIDE1{A.[D*].C}$$$$V2.0",
    ),
    "biln_large_bond_id": (
        "A-C(7563,3).C(7563,3)",
        "PEPTIDE1{A.C}|PEPTIDE2{C}$PEPTIDE1,PEPTIDE2,2:R3-1:R3$$$V2.0",
    ),
    "biln_explicit_backbone": (
        "[D-Cit](1,2).aThr(1,1)(2,2).meS(2,1)",
        "PEPTIDE1{[D-Cit].[aThr].[meS]}$$$$V2.0",
    ),
    "biln_disulfides": (
        "D-T-H-F-P-I-C(1,3)-I-F-C(2,3)-C(3,3)-G-C(2,3)-C(4,3)-H-R-S-K-C(3,3)-G-M-C(4,3)-C(1,3)-K-T",
        "PEPTIDE1{D.T.H.F.P.I.C.I.F.C.C.G.C.C.H.R.S.K.C.G.M.C.C.K.T}$PEPTIDE1,PEPTIDE1,7:R3-23:R3|PEPTIDE1,PEPTIDE1,10:R3-13:R3|PEPTIDE1,PEPTIDE1,11:R3-19:R3|PEPTIDE1,PEPTIDE1,14:R3-22:R3$$$V2.0",
    ),
}

for name in sorted(biln_to_helm.keys()):
    biln, helm_ref = biln_to_helm[name]
    try:
        doc = indigo.loadBiln(biln, lib)
        helm = doc.helm(lib)
        diff = find_diff(helm_ref, helm)
        if diff:
            print(name + ":FAILED")
            print(diff)
        else:
            print(name + ":BILN->HELM SUCCEED")
    except IndigoException as e:
        print(name + ":FAILED - " + getIndigoExceptionText(e))

biln_to_biln = {
    "biln_cycle_best": (
        "A(1,1)-C-D-E(1,2)",
        "A(1,1)-C-D-E(1,2)",
    ),
    "biln_cycle_rotation": (
        "C(1,1)-D-E-A(1,2)",
        "A(1,1)-C-D-E(1,2)",
    ),
    "biln_cycle_reverse_rotation": (
        "D(1,2)-C-A-E(1,1)",
        "A(1,1)-C-D-E(1,2)",
    ),
    "biln_bracketed_no_hyphen": (
        "[D-2Thi]-[D]-[D-gGlu]-[meF]-[G]-[Lys-al]",
        "[D-2Thi]-D-[D-gGlu]-meF-G-[Lys-al]",
    ),
    "biln_backbone_order": (
        "A-A-A-A-A-A.C-C-C-C.[PEG-2]-C-C-C-C-[PEG-2]",
        "A-A-A-A-A-A.[PEG-2]-C-C-C-C-[PEG-2].C-C-C-C",
    ),
    "biln_short_chain_order": (
        "A-A-A6OH-A6OH-A6OH.C-C-C-C-C",
        "A-A-A6OH-A6OH-A6OH.C-C-C-C-C",
    ),
    "biln_amino_acid_count_order": (
        "C-C-A6OH-A6OH-C-C.C-C-C-C-C-A6OH",
        "C-C-C-C-C-A6OH.C-C-A6OH-A6OH-C-C",
    ),
    "biln_alphabetic_order": (
        "C-D-E-F-G-A6OH.A-C-D-E-F-A6OH",
        "A-C-D-E-F-A6OH.C-D-E-F-G-A6OH",
    ),
    "biln_multiple_nonbackbone_order": (
        "A-[Test-6-Ch](1,4)(2,3)-C.D(2,1).E(1,2)",
        "A-[Test-6-Ch](1,3)(2,4)-C.D(1,1).E(2,2)",
    ),
    "biln_cycle_with_extra_bond_order": (
        "C(1,1)(2,3)-C-C(2,3)-C(1,2)",
        "C(1,1)-C(2,3)-C-C(1,2)(2,3)",
    ),
    "biln_library_alias": (
        "Edc",
        "Edc",
    ),
    "biln_valid_large_bond_ids": (
        "A-C(7563,3)-D(3,3)-E.F-G-H(7563,3)-I-K(3,3)",
        "F-G-H(1,3)-I-K(2,3).A-C(1,3)-D(2,3)-E",
    ),
}

for name in sorted(biln_to_biln.keys()):
    biln, biln_ref = biln_to_biln[name]
    try:
        doc = indigo.loadBiln(biln, lib)
        canonical_biln = doc.biln(lib)
        diff = find_diff(biln_ref, canonical_biln)
        if diff:
            print(name + ":FAILED")
            print(diff)
        else:
            print(name + ":BILN->BILN SUCCEED")
    except IndigoException as e:
        print(name + ":FAILED - " + getIndigoExceptionText(e))

helm_to_biln = {
    "helm_underscore_alias": (
        "PEPTIDE1{A.[1Nal].[Cys_Bn].C}$$$$V2.0",
        "A-1Nal-Cys_Bn-C",
    ),
    "helm_bracketed_alias": (
        "PEPTIDE1{A.[D-1Nal].[Cys_Bn].C}$$$$V2.0",
        "A-[D-1Nal]-Cys_Bn-C",
    ),
    "helm_star_alias": (
        "PEPTIDE1{A.[D*].C}$$$$V2.0",
        "A-D*-C",
    ),
    "helm_cycle": (
        "PEPTIDE1{A.C.D.E}$PEPTIDE1,PEPTIDE1,1:R1-4:R2$$$V2.0",
        "A(1,1)-C-D-E(1,2)",
    ),
    "helm_chem_backbone": (
        "PEPTIDE1{A.A.A.A.A.A}|CHEM1{[PEG-2]}|PEPTIDE2{C.C.C.C}|CHEM2{[PEG-2]}$PEPTIDE1,CHEM1,6:R2-1:R1|CHEM1,PEPTIDE2,1:R2-1:R1|PEPTIDE2,CHEM2,4:R2-1:R1$$$V2.0",
        "A-A-A-A-A-A-[PEG-2]-C-C-C-C-[PEG-2]",
    ),
    "helm_chem_with_biln_code": (
        "CHEM1{[PEG-2]}$$$$V2.0",
        "[PEG-2]",
    ),
    "helm_alias_to_biln_alias": (
        "PEPTIDE1{[Cys_SEt]}$$$$V2.0",
        "Edc",
    ),
}

for name in sorted(helm_to_biln.keys()):
    helm, biln_ref = helm_to_biln[name]
    try:
        doc = indigo.loadHelm(helm, lib)
        biln = doc.biln(lib)
        diff = find_diff(biln_ref, biln)
        if diff:
            print(name + ":FAILED")
            print(diff)
        else:
            print(name + ":HELM->BILN SUCCEED")
    except IndigoException as e:
        print(name + ":FAILED - " + getIndigoExceptionText(e))

helm_errors = {
    "CHEM1{[qweqwe]}$$$$V2.0": "Only amino acids and CHEMs with BILN codes can get exported to BILN.",
    "PEPTIDE1{A}|RNA1{R(A)P}$$$$V2.0": "Only amino acids and CHEMs with BILN codes can get exported to BILN.",
}

for helm in sorted(helm_errors.keys()):
    error = helm_errors[helm]
    try:
        doc = indigo.loadHelm(helm, lib)
        doc.biln(lib)
        print("Test %s failed: exception expected." % helm)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (helm, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (helm, error, text)
            )

ket_errors = {
    "custom_chem_without_biln_code": (
        "CHEM1{[qweqwe]}$$$$V2.0",
        "Only amino acids and CHEMs with BILN codes can get exported to BILN.",
    ),
}

for name in sorted(ket_errors.keys()):
    helm, error = ket_errors[name]
    try:
        doc = indigo.loadHelm(helm, lib)
        doc = indigo.loadKetDocument(doc.json())
        doc.biln(lib)
        print("Test KET %s failed: exception expected." % name)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test KET '%s': got expected error '%s'" % (name, error))
        else:
            print(
                "Test KET '%s': expected error '%s' but got '%s'"
                % (name, error, text)
            )

biln_errors = {
    "A(1,3)-C": "The string cannot be interpreted as a valid BILN string.",
    "A--C": "The string cannot be interpreted as a valid BILN string.",
    "A-C(1,4)": "The string cannot be interpreted as a valid BILN string.",
    "[D-Cit](1,2)-aThr(1,1)(2,2)-meS(2,1)": "The string cannot be interpreted as a valid BILN string.",
    "D-2Thi-D-D-gGlu-meF-G-Lys-al": "The string cannot be interpreted as a valid BILN string.",
    "A-C(-1,3)-D(2,3)-E.F-G-H(-1,3)-I-K(2,3)": "The string cannot be interpreted as a valid BILN string.",
    "A-C(1.25,3)-D(2,3)-E.F-G-H(1.25,3)-I-K(2,3)": "The string cannot be interpreted as a valid BILN string.",
    "A-C(1,3)-D(1,3)-E.F-G-H(1,3)-I-K(2,3)": "The string cannot be interpreted as a valid BILN string.",
    "A-C(1,4)-D(2,3)-E.F-G-H(1,3)-I-K(2,3)": "The string cannot be interpreted as a valid BILN string.",
    "Cys_SEt": "The string cannot be interpreted as a valid BILN string.",
}

for biln in sorted(biln_errors.keys()):
    error = biln_errors[biln]
    try:
        doc = indigo.loadBiln(biln, lib)
        print("Test %s failed: exception expected." % biln)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (biln, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (biln, error, text)
            )
