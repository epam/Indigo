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
    "idt_a": "A",
    "idt_acg": "ACG",
    "idt_maxmgc": "mA*mGC",
    "idt_2moera": "/52MOErA//i2MOErA//32MOErA/",
    "idt_52moera": "/52MOErA/",
    "idt_32moera": "/32MOErA/",
    "idt_modifications": "/5Phos//i2MOErC//3Phos/",
}

lib = indigo.loadMoleculeFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename, idt in idt_data.items():
    mol = indigo.loadIdt(idt)
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
    "!+-$#12w12r23e32e33": "SEQUENCE loader: SequenceLoader::loadIdt(), Invalid symbols in the sequence: !,-,$,#,1,2,w,1,2,2,3,e,3,2,e,3,3",
    "/": "SEQUENCE loader: Unexpected end of data",
    "//": "SEQUENCE loader: Invalid modification: empty string.",
    "/a/": "SEQUENCE loader: Invalid modification: a.",
    "A*": "SEQUENCE loader: Invalid IDT sequence: '*' couldn't be the last symbol.",
    "/i2MOErQA/": "SEQUENCE loader: IDT alias i2MOErQA not found at three-prime end position.",
}
for idt_seq, error in idt_errors.items():
    try:
        mol = indigo.loadIdt(idt_seq)
        print("Test %s failed: exception expected." % idt_seq)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if text == error:
            print("Test '%s': got expected error '%s'" % (idt_seq, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (idt_seq, error, text)
            )
