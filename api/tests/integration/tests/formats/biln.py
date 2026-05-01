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
    "biln_cap": (
        "Ac(1,2).A-K(1,3)",
        "PEPTIDE1{[Ac]}|PEPTIDE2{A.K}$PEPTIDE1,PEPTIDE2,1:R2-2:R3$$$V2.0",
    ),
    "biln_three_chains": (
        "Ac(1,2).A-K(1,3)(2,2).Me(2,1)",
        "PEPTIDE1{[Ac]}|PEPTIDE2{A.K}|PEPTIDE3{[Me]}$PEPTIDE1,PEPTIDE2,1:R2-2:R3|PEPTIDE2,PEPTIDE3,2:R2-1:R1$$$V2.0",
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

helm_to_biln = {
    "helm_cap": (
        "PEPTIDE1{[Ac]}|PEPTIDE2{A.K}$PEPTIDE1,PEPTIDE2,1:R2-2:R3$$$V2.0",
        "Ac(1,2).A-K(1,3)",
    ),
    "helm_three_chains": (
        "PEPTIDE1{[Ac]}|PEPTIDE2{A.K}|PEPTIDE3{[Me]}$PEPTIDE1,PEPTIDE2,1:R2-2:R3|PEPTIDE2,PEPTIDE3,2:R2-1:R1$$$V2.0",
        "Ac(1,2).A-K(1,3)(2,2).Me(2,1)",
    ),
    "helm_cycle": (
        "PEPTIDE1{[Abu].[Sar].[NMeL].V.[NMeL].A.[DAla].[NMeL].[NMeL].[NMeV].[NMeThr4RBut2enyl]}$PEPTIDE1,PEPTIDE1,1:R1-11:R2$$$V2.0",
        "Abu(1,1)-Sar-NMeL-V-NMeL-A-DAla-NMeL-NMeL-NMeV-NMeThr4RBut2enyl(1,2)",
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

biln_errors = {
    "A(1,3)-C": "Invalid BILN bond 1: expected two endpoints but found 1.",
    "A--C": "Invalid BILN string: empty monomer.",
    "A-C(1,4)": "Invalid BILN bond 1: expected two endpoints but found 1.",
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
