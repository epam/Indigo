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

print("*** KET to AxoLabs ***")

root = joinPathPy("molecules/", __file__)
ref = joinPathPy("ref/", __file__)

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

# same ref ket files used to check axolabs-to-ket and to check ket-to-axolabs
axolabs_data = {
    "AxoLabs_pACsGsUp": "5'-pACsGsUp-3'",
    "AxoLabs_ACsGsU": "5'-ACsGsU-3'",
    "AxoLabs_chem": "5'-dI(5MdC)AmA(NHC6)GmTm-3'",
    "AxoLabs_brackets": "5'-(5MdC)(5MdC)(5MdC)-3'",
}

for filename in sorted(axolabs_data.keys()):
    mol = indigo.loadKetDocumentFromFile(os.path.join(ref, filename + ".ket"))
    try:
        axolabs = mol.axolabs(lib)
        axolabs_ref = axolabs_data[filename]
        if axolabs_ref == axolabs:
            print(filename + ".ket:SUCCEED")
        else:
            print(
                "%s FAILED : expected '%s', got '%s'"
                % (filename, axolabs_ref, axolabs)
            )
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        print(filename + ".ket:FAILED - %s" % text)

axolabs_errors = {
    "ket-to-idt-r1r1connection": "Cannot save in AxoLabs format - non-standard connection found.",
    "ket-to-idt-peptide": "Cannot save molecule in AxoLabs format - expected sugar but found AminoAcid monomer DPhe4C.",
    "ket-to-idt-two-bases": "Cannot save in AxoLabs format - non-standard connection found.",
    "ket-to-idt-invalid-posphates": "Cannot save in AxoLabs format - non-standard connection found.",
    "ket-to-idt-invalid-nucleotide": "Cannot save molecule in AxoLabs format - non-standard phosphate 'mepo2' found",
    "ket-to-idt-invalid-sugar-phosphate": "Cannot save molecule in AxoLabs format - non-standard phosphate 'mepo2' found",
    "ket-to-idt-invalid-sugar": "Sugar:m2e2r has no AxoLabs alias.",
    "ket-to-idt-invalid-sugar-base": "Group sugar:m2e2r base:z8c3G not found.",
    "ket-to-idt-alternatives-base": "Cannot save in AxoLabs format - ambiguous base 'S' found.",
    "ket-to-idt-invalid-last-phosphate": "Cannot save molecule in AxoLabs format - phosphate sP cannot be last monomer in sequence.",
    "ket-to-idt-no3prime": "Nucleotide '5Br-dU' has no AxoLabs alias.",
}
for filename in sorted(axolabs_errors.keys()):
    error = axolabs_errors[filename]
    try:
        doc = indigo.loadKetDocumentFromFile(
            os.path.join(root, filename + ".ket")
        )
        axolabs = doc.axolabs(lib)
        print(
            "Test %s failed: exception expected but got next axolabs - '%s'."
            % (filename, axolabs)
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
