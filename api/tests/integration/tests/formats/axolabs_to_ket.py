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

print("*** AxoLabs to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

axolabs_data = {
    "AxoLabs_pACsGsUp": "5'-pACsGsUp-3'",
    "AxoLabs_ACsGsU": "5'-ACsGsU-3'",
    "AxoLabs_chem": "5'-dI(5MdC)AmA(NHC6)GmTm-3'",
    "AxoLabs_brackets": "5'-(5MdC)(5MdC)(5MdC)-3'",
    "AxoLabs_unknown": "5'-A(Unknown1)(Unknown2)(Unknown3)A-3'",
    "AxoLabs_double": "5'-AC-3'\n5'-GU-3'",
    "AxoLabs_double_and_single": "5'-AC-3'\n5'-GU-3'\n5'-UG-3'",
    "AxoLabs_two_double": "5'-AC-3'\n5'-GU-3'\n5'-AC-3'\n5'-GU-3'",
    "AxoLabs_double_sense_shift": "5'-AC-3'\n5'-GUAA-3'",
    "AxoLabs_double_antisense_shift": "5'-AAAC-3'\n5'-GU-3'",
    "AxoLabs_double_3": "5'-TmGCGCA-3'\n5'-UCAA-3'",
}

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename in sorted(axolabs_data.keys()):
    mol = indigo.loadAxoLabs(axolabs_data[filename], lib)
    ket = mol.json()
    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
        ket_ref = file.read()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)

axolabs_errors = {
    "5'-pACsGsUsp-3'": "Invalid AxoLabs sequence: phosphate 's' can only be internal",
    "5'-sACsGsUp-3'": "Invalid AxoLabs sequence: phosphate 's' can only be internal",
    "5'-pACsGsUs-3'": "Invalid AxoLabs sequence: phosphate 's' can only be internal",
    "5-pACsGsUsp-3'": "Invalid AxoLabs sequence: expected 5'- got 5-p",
    "5'-pACsGsUsp-3": "Invalid AxoLabs sequence: expected -3' got p-3",
    "5'-pACpGsUsp-3'": "Invalid AxoLabs sequence: phosphate 'p' can only be terminal",
}
for axolabs_seq in sorted(axolabs_errors.keys()):
    error = axolabs_errors[axolabs_seq]
    try:
        mol = indigo.loadAxoLabs(axolabs_seq, lib)
        print("Test %s failed: exception expected." % axolabs_seq)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (axolabs_seq, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (axolabs_seq, error, text)
            )
