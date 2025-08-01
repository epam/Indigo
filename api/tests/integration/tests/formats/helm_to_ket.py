﻿import difflib
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
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** HELM to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

helm_data = {
    "helm_simple_rna": "RNA1{r(U)p.r(T)p.r(G)p.r(C)p.r(A)}$$$$V2.0",
    "helm_multi_char_rna": "RNA1{r(U)p.r(T)p.r(G)p.r(C)p.r([m62A])}$$$$V2.0",
    "helm_peptide": "PEPTIDE1{A.[meA].C}$$$$V2.0",
    "helm_chem_peptide": "CHEM1{[PEG-2]}|PEPTIDE1{W.N.D.[Pen].G.[Orn].D.A.D.G.S.G.[Cap]}$CHEM1,PEPTIDE1,1:R1-1:R1$$$V2.0",
    "helm_annotations": 'BLOB1{BEAD}"Animated Polystyrene"|CHEM1{[hxy]"Annotation"}|RNA1{R(A"mutation")P.R(U)P.R(G)P}$$$$V2.0',
    "helm_chem_rna": "CHEM1{[MCC]}|RNA1{R(U)P}$CHEM1,RNA1,1:R1-3:R2$$$V2.0",
    "helm_rna_without_base": "RNA1{R.P}$$$$V2.0",
    "helm_mixed_base": "RNA1{d(A)P.d(A+G)P.d(A)P.d(G+C)}$$$$V2.0",
    "helm_mixed_custom": "RNA1{d(A:10+[Xan]:20+G:30+T:50)P.d(A:10+C:20+G:30+T:50)P.d(A+C+G+T)}$$$$V2.0",
    "helm_aminoacids_variants": "PEPTIDE1{([Dha]+N).(L+I).(E+Q).(A+C+D+E+F+G+H+I+K+L+M+N+O+P+Q+R+S+T+U+V+W+Y)}$$$$V2.0",
    "helm_smiles": "PEPTIDE1{G.[[*]N[C@@H](C=O)C([*])=O |$_R1;;;;;;_R2;$|].C}|PEPTIDE2{G.[[*:1]N[C@@H](C=O)C([*:2])=O].C}$$$$",
    "helm_smiles_sugar": "RNA1{[C([*:3])[C@@H](O[*:2])CO[*:1]](A)P}$$$$V2.0",
    "helm_no_left_ap": "PEPTIDE1{[DACys].C}$$$$V2.0",
    "helm_fractional_ratio": "PEPTIDE1{(A:1.5+C:0.1+G:3)}$$$$V2.0",
    "helm_chem_rna_hydro": "CHEM1{[MCC]}|RNA1{R(U)P}$CHEM1,RNA1,1:pair-3:pair$$$V2.0",
    "helm_unsplit": "RNA1{[5Br-dU]}$$$$V2.0",
    "helm_smiles_no_ap": "CHEM1{[P(O)(O)(=O)O]}$$$$V2.0",
    "helm_any_chem": "CHEM1{*}|CHEM2{*}$$$$V2.0",
    "helm_2818": "RNA1{r(A,C,G,U)p.r(C,G,U)p.r(A,G,U)p.r(A,C,U)p.r(G,U)p.r(A,U)p.r(C,U)p}$$$$V2.0",
    "helm_2826": "RNA1{d(A,C,G,T)p.d(A,G,T)p.d(A,T)P}|RNA2{r(A,C,G,U)p.r(A,C,U)p.r(A,U)[Ssp]}|RNA3{[RSpabC](A,U)p}$RNA1,RNA2,2:pair-8:pair|RNA1,RNA2,5:pair-5:pair|RNA2,RNA1,2:pair-8:pair$$$V2.0",
    "helm_alias": "RNA1{m(A)P.[n3r](C)P}$$$$V2.0",
    "helm_no_brackets": "PEPTIDE1{DACys.C.(meA+C)}$$$$V2.0",
    "helm_unresolved": "PEPTIDE1{Unres1.Unres2.(Unres1+Unres2+Unres3)}|RNA1{Unres1(Unres2)}$$$$V2.0",
    "helm_unresolved_rna": "RNA1{Sugar1(Base1)Phos3.Rna1.Rna2}$$$$V2.0",
    "helm_only_base": "RNA1{(A)}$$$$V2.0",
    "helm_nucleosides": "RNA1{[5R6Rm5](A).[5R6Rm5](A).[5R6Rm5](A)}$$$$V2.0",
}

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename in sorted(helm_data.keys()):
    try:
        mol = indigo.loadHelm(helm_data[filename], lib)
        # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
        #     file.write(mol.json())
        with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
            ket_ref = file.read()
        ket = mol.json()
        diff = find_diff(ket_ref, ket)
        if not diff:
            print(filename + ".ket:SUCCEED")
        else:
            print(filename + ".ket:FAILED")
            print(diff)
            print(ket)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        print(filename + ".ket:FAILED - " + text)

helm_errors = {
    "PEPTIDE1{A'2'}$$$$V2.0": "Repeating not supported now.",
    "CHEM1{[MCC]}|RNA1{R(A)P.R(C)P.R(G)P.R(T)P.R(U)P}$RNA1,PEPTIDE1,15:R2-1:R1$$$V2.0": "Polymer 'PEPTIDE1' not found.",
    "CHEM1{[A6OH]}|PEPTIDE1{A}$CHEM10,PEPTIDE1,1:R2-1:R1$$$V2.0": "Polymer 'CHEM10' not found.",
    "CHEM1{[A6OH]}|PEPTIDE1{A}$CHEM1,PEPTIDE1,1:R2-3:R1$$$V2.0": "Polymer 'PEPTIDE1' does not contains monomer with number 3.",
    "CHEM1{[A6OH]}|PEPTIDE1{A}$CHEM1,PEPTIDE1,1:R4-1:R1$$$V2.0": "Unknown attachment point 'R4' in monomer 'A6OH(monomer0)'",
    "CHEM1{[MCC]}|RNA1{R(U)P}$CHEM1,RNA1,1:R1-1:R2$$$V2.0": "Monomer 'R(monomer1)' attachment point 'R2' already connected to monomer'monomer3' attachment point 'R1'",
    "PEPTIDE1{(A:1.5+C:aaaa)}$$$$V2.0": "Unexpected symbol. Expected digit but found 'a'",
    "PEPTIDE1{(A:+C:0.1)}$$$$V2.0": "Unexpected symbol. Expected digit but found '+'",
    "PEPTIDE1{(A:1.5.+C:0.1)}$$$$V2.0": "Enexpected symbol. Second dot in number",
    "RNA1{R[P(O)(O)(=O)O]}$$$$V2.0": "Unknown attachment point 'R1' in monomer 'Mod0(monomer1)'",
    "PEPTIDE1{1Nal]}$$$$V2.0": "SEQUENCE loader: Unexpected symbol ']'.",
}
for helm_seq in sorted(helm_errors.keys()):
    error = helm_errors[helm_seq]
    try:
        mol = indigo.loadHelm(helm_seq, lib)
        print("Test %s failed: exception expected." % helm_seq)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (helm_seq, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (helm_seq, error, text)
            )
