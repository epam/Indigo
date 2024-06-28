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
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** HELM to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

helm_data = {
    "helm_simple_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R(A)}$$$$V2.0",
    "helm_multi_char_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R([daA])}$$$$V2.0",
    "helm_peptide": "PEPTIDE1{A.[meA].C}$$$$V2.0",
    "helm_chem_peptide": "CHEM1{[PEG2]}|PEPTIDE1{W.N.D.[Pen].G.[Orn].D.A.D.G.S.G.[Cap]}$CHEM1,PEPTIDE1,1:R1-1:R1$$$V2.0",
    "helm_annotations": 'BLOB1{BEAD}"Animated Polystyrene"|CHEM1{[hxy]"Annotation"}|RNA1{R(A"mutation")P.R(U)P.R(G)P}$$$$V2.0',
}

lib = indigo.loadMoleculeFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename in sorted(helm_data.keys()):
    mol = indigo.loadHelm(helm_data[filename])
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

helm_errors = {"PEPTIDE1{A'2'}$$$$V2.0": "Repeating do not supported now."}
for helm_seq in sorted(helm_errors.keys()):
    error = helm_errors[helm_seq]
    try:
        mol = indigo.loadHelm(helm_seq)
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
