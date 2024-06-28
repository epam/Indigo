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

indigo.loadMoleculeFromFile(os.path.join(ref, "monomer_library.ket"))

# same ref ket files used to check idt-to-ket and to check ket-to-idt
helm_data = {
    "helm_simple_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R(A)}$$$$V2.0",
    "helm_multi_char_rna": "RNA1{R(U)P.R(T)P.R(G)P.R(C)P.R([daA])}$$$$V2.0",
    "helm_peptide": "PEPTIDE1{A.[meA].C}$$$$V2.0",
    "helm_chem_peptide": "CHEM1{[PEG2]}|PEPTIDE1{W.N.D.[Pen].G.[Orn].D.A.D.G.S.G.[Cap]}$CHEM1,PEPTIDE1,1:R1-1:R0$$$V2.0",
    "helm_annotations": "CHEM1{[hxy]}|RNA1{R(A)P.R(U)P.R(G)P}$$$$V2.0",
}

for filename in sorted(helm_data.keys()):
    mol = indigo.loadMoleculeFromFile(os.path.join(ref, filename + ".ket"))
    idt = mol.helm()
    idt_ref = helm_data[filename]
    if idt_ref == idt:
        print(filename + ".ket:SUCCEED")
    else:
        print(
            "%s.idt FAILED : expected '%s', got '%s'"
            % (filename, idt_ref, idt)
        )

helm_errors = {}
for filename in sorted(helm_errors.keys()):
    error = helm_errors[filename]
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
        idt = mol.idt()
        print(
            "Test %s failed: exception expected but got next idt - '%s'."
            % (filename, idt)
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
