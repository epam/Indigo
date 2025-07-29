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
    "helm_simple_rna": "RNA1{r(U)p.r(T)p.r(G)p.r(C)p.r(A)}$$$$V2.0",
    "helm_multi_char_rna": "RNA1{r(U)p.r(T)p.r(G)p.r(C)p.r([m62A])}$$$$V2.0",
    "helm_peptide": "PEPTIDE1{A.[meA].C}$$$$V2.0",
    "helm_chem_peptide": "CHEM1{[PEG-2]}|PEPTIDE1{W.N.D.[Pen].G.[Orn].D.A.D.G.S.G.[Cap]}$CHEM1,PEPTIDE1,1:R1-1:R1$$$V2.0",
    "helm_annotations": "CHEM1{[hxy]}|RNA1{r(A)p.r(U)p.r(G)p}$$$$V2.0",
    "helm_connetion_separator": "PEPTIDE1{A}|PEPTIDE2{C}|CHEM1{[A6OH]}$PEPTIDE1,CHEM1,1:R2-1:R1|CHEM1,PEPTIDE2,1:R2-1:R1$$$V2.0",
    "helm_chem_rna": "CHEM1{[MCC]}|RNA1{r(U)p}$CHEM1,RNA1,1:R1-3:R2$$$V2.0",
    "helm_rna_without_base": "RNA1{r.p}$$$$V2.0",
    "helm_cycled_polymer": "PEPTIDE1{A.C.D.E.F}$PEPTIDE1,PEPTIDE1,5:R2-1:R1$$$V2.0",
    "helm_mixed_base": "RNA1{d(A)p.d(A+G)p.d(A)p.d(G+C)}$$$$V2.0",
    "helm_mixed_custom": "RNA1{d(A:10+[Xan]:20+G:30+T:50)p.d(A:10+C:20+G:30+T:50)p.d(A+C+G+T)}$$$$V2.0",
    "helm_aminoacids_variants": "PEPTIDE1{([Dha]+N).(L+I).(E+Q).(A+C+D+E+F+G+H+I+K+L+M+N+O+P+Q+R+S+T+U+V+W+Y)}$$$$V2.0",
    "dna_variants": "RNA1{d(C,G,T)p.d(A,C,G,T)}$$$$V2.0",
    "rna_variants": "RNA1{r(A,G)p.r(G,U)p.r(A,C,G,U)}$$$$V2.0",
    "helm_monomer_molecule": "PEPTIDE1{A}|PEPTIDE2{G}|CHEM1{[C(N[*:2])=C[*:1] |$;;_R2;;_R1$|]}$CHEM1,PEPTIDE1,1:R2-1:R1|PEPTIDE2,CHEM1,1:R2-1:R1$$$V2.0",
    "helm_fractional_ratio": "PEPTIDE1{(A:1.5+C:0.1+G:3)}$$$$V2.0",
    "helm_smiles": "PEPTIDE1{G.[[*:1]NC(C(=O)[*:2])C=O |$_R1;;;;;_R2;;$|].C}|PEPTIDE2{G.[[*:1]NC(C(=O)[*:2])C=O |$_R1;;;;;_R2;;$|].C}$$$$V2.0",
    "helm_smiles_sugar": "RNA1{[C(C(CO[*:1])O[*:2])[*:3] |$;;;;_R1;;_R2;_R3$|](A)p}$$$$V2.0",
    "helm_molecule_2418": "PEPTIDE1{A}|CHEM1{[C1C=CC=CC=1[*:1] |$;;;;;;_R1$|]}$PEPTIDE1,CHEM1,1:R1-1:R1$$$V2.0",
    "helm_chem_rna_hydro": "CHEM1{[MCC]}|RNA1{r(U)p}$CHEM1,RNA1,1:pair-3:pair$$$V2.0",
    "helm_monomer_molecule_direct": "PEPTIDE1{A}|CHEM1{[C(=C)N[*:1] |$;;;_R1$|]}$PEPTIDE1,CHEM1,1:R2-1:R1$$$V2.0",
    "helm_unknown": "CHEM1{*}$$$$V2.0",
    "helm_different_id": "PEPTIDE1{A}|RNA1{r(A)p}$$$$V2.0",
    "helm_any_chem": "CHEM1{*}|CHEM2{*}$$$$V2.0",
    "helm_unresolved": "PEPTIDE1{[Unres1].[Unres2].([Unres1]+[Unres2]+[Unres3])}|RNA1{[Unres1]([Unres2])}$$$$V2.0",
    "helm_unresolved_rna": "RNA1{[Sugar1]([Base1])[Phos3].[Rna1].[Rna2]}$$$$V2.0",
    "helm_nucleosides": "RNA1{[5R6Rm5cEt](A).[5R6Rm5cEt](A).[5R6Rm5cEt](A)}$$$$V2.0",
}

for filename in sorted(helm_data.keys()):
    mol = indigo.loadKetDocumentFromFile(os.path.join(ref, filename + ".ket"))
    try:
        helm = mol.helm(lib)
    except IndigoException as e:
        print("Test %s failed: %s" % (filename, getIndigoExceptionText(e)))
        continue
    except Exception as e:
        print("Test %s failed: %s" % (filename, e))
        continue

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
