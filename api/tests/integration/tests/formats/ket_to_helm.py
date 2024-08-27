import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to HELM ***")

root = joinPathPy("molecules/", __file__)
ref = joinPathPy("ref/", __file__)

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

# same ref ket files used to check helm-to-ket and to check ket-to-helm
helm_data = {
    "helm_simple_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R(A)}$$$$V2.0",
    "helm_multi_char_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R([daA])}$$$$V2.0",
    "helm_peptide": "PEPTIDE1{A.[meA].C}$$$$V2.0",
    "helm_chem_peptide": "CHEM1{[PEG2]}|PEPTIDE1{W.N.D.[Pen].G.[Orn].D.A.D.G.S.G.[Cap]}$CHEM1,PEPTIDE1,1:R1-1:R1$$$V2.0",
    "helm_annotations": "CHEM1{[hxy]}|RNA1{R(A)P.R(U)P.R(G)P}$$$$V2.0",
    "helm_connetion_separator": "PEPTIDE1{A}|PEPTIDE2{C}|CHEM1{[A6OH]}$PEPTIDE1,CHEM1,1:R2-1:R1|CHEM1,PEPTIDE2,1:R2-1:R1$$$V2.0",
    "helm_chem_rna": "CHEM1{[MCC]}|RNA1{R(U)P}$CHEM1,RNA1,1:R1-3:R2$$$V2.0",
    "helm_rna_without_base": "RNA1{RP}$$$$V2.0",
    "helm_cycled_polymer": "PEPTIDE1{A.C.D.E.F}$PEPTIDE1,PEPTIDE1,5:R2-1:R1$$$V2.0",
    "helm_mixed_base": "RNA1{[dR](A)P.[dR](A+G)P.[dR](A)P.[dR](G+C)}$$$$V2.0",
    "helm_mixed_custom": "RNA1{[dR](A:10+C:20+G:30+T:50)P.[dR](A:10+C:20+G:30+T:50)P.[dR](A+C+G+T)}$$$$V2.0",
    "aminoacids_variants": "PEPTIDE1{(D+N).(L+I).(E+Q).(A+C+D+E+F+G+H+I+K+L+M+N+O+P+Q+R+S+T+U+V+W+Y)}$$$$V2.0",
    "dna_variants": "RNA1{[dR](C+G+T)P.[dR](A+C+G+T)}$$$$V2.0",
    "rna_variants": "RNA1{R(G+T)P.R(A+C+G+T)}$$$$V2.0",
}

for filename in sorted(helm_data.keys()):
    mol = indigo.loadKetDocumentFromFile(os.path.join(ref, filename + ".ket"))
    helm = mol.helm(lib)
    helm_ref = helm_data[filename]
    if helm_ref == helm:
        print(filename + ".ket:SUCCEED")
    else:
        print(
            "%s.ket FAILED : expected '%s', got '%s'"
            % (filename, helm_ref, helm)
        )

helm_errors = {}
for filename in sorted(helm_errors.keys()):
    error = helm_errors[filename]
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
        helm = mol.helm(lib)
        print(
            "Test %s failed: exception expected but got next helm - '%s'."
            % (filename, helm)
        )
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test %s: got expected error '%s'" % (filename, error))
        else:
            print(
                "Test %s: expected error '%s' but got '%s'"
                % (filename, error, text)
            )
