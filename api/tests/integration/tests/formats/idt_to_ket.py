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

print("*** IDT to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

idt_data = {
    "idt_single_nucleoside": "A",
    "idt_bases": "ATCGUI",
    "idt_prefix_suffix": "mA*rT*+C*G*+UrImA",
    "idt_modifications": "/52MOErA/*/i2MOErA//32MOErA/",
    "idt_52moera_with_3phos": "/52MOErA//3Phos/",
    "idt_singe_32moera_nucleoside": "/32MOErA/",
    "idt_std_phosphates": "/5Phos/ATG/3Phos/",
    "idt_mod_phosphates": "/5Phos//i2MOErC//3Phos/",
    "idt_mixed": "/5Phos/+A*/i2MOErA/*rG/3Phos/",
    "idt_many_molecules": "ACTG\n/52MOErA/*AU/3Phos/\rAC/i2MOErC//3Phos/\n\rTACG",
}

lib = indigo.loadMoleculeFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename in sorted(idt_data.keys()):
    mol = indigo.loadIdt(idt_data[filename])
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

idt_errors = {
    "!+-$#12w12r23e32e33": "Invalid symbols in the sequence: !,-,$,#,1,2,w,1,2,2,3,e,3,2,e,3,3",
    "/": "Unexpected end of data",
    "//": "Invalid modification: empty string.",
    "/a/": "Invalid modification: a.",
    "A*": "Invalid IDT sequence: '*' couldn't be the last symbol.",
    "/i2MOErA/": "IDT alias i2MOErA not found at three-prime end position.",
    "/i2MOErA/T": "IDT alias i2MOErA not found at five-prime end position.",
    "T/i2MOErA/": "IDT alias i2MOErA not found at three-prime end position.",
    "/32MOErA/T": "IDT alias 32MOErA not found at five-prime end position.",
    "/52MOErA/": "IDT alias 52MOErA not found at three-prime end position.",
    "T/52MOErA/": "IDT alias 52MOErA not found at three-prime end position.",
    "/3Phos/T": "IDT alias 3Phos not found at three-prime end position.",
    "T/5Phos/": "IDT alias 5Phos not found at three-prime end position.",
    "/5Phos/*A": "/5Phos/ cannot be modified to 'sP'",
    "r/5Phos/A": "Sugar prefix could not be used with '/5Phos/'",
    "+/5Phos/A": "Sugar prefix could not be used with '/5Phos/'",
    "m/5Phos/A": "Sugar prefix could not be used with '/5Phos/'",
    "Ar/3Phos/": "Sugar prefix could not be used with '/3Phos/'",
    "A+/3Phos/": "Sugar prefix could not be used with '/3Phos/'",
    "Am/3Phos/": "Sugar prefix could not be used with '/3Phos/'",
}
for idt_seq in sorted(idt_errors.keys()):
    error = idt_errors[idt_seq]
    try:
        mol = indigo.loadIdt(idt_seq)
        print("Test %s failed: exception expected." % idt_seq)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (idt_seq, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (idt_seq, error, text)
            )
