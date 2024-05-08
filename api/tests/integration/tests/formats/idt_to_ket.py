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

idt_names = [
    "idt_a",
    "idt_acg",
    "idt_maxmgc",
    "idt_2moera",
    "idt_modifications",
]

idt_data = {
    "idt_a": "A",
    "idt_acg": "ACG",
    "idt_maxmgc": "mA*mGC",
    "idt_2moera": "/52MOErA//i2MOErA//32MOErA/",
    "idt_modifications": "/5Phos//i2MOErC//3Phos/",
}

lib = indigo.loadMoleculeFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for idt_name in idt_names:
    mol = indigo.loadIdt(idt_data[idt_name])
    # with open(os.path.join(ref_path, idt_name) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(os.path.join(ref_path, idt_name) + ".ket", "r") as file:
        ket_ref = file.read()
    ket = mol.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(idt_name + ".ket:SUCCEED")
    else:
        print(idt_name + ".ket:FAILED")
        print(diff)

idt_errors = {
    "!+-$#12w12r23e32e33": "SEQUENCE loader: SequenceLoader::loadIdt(), Invalid symbols in the sequence: !,-,$,#,1,2,w,1,2,2,3,e,3,2,e,3,3"
}
for idt_seq, error in idt_errors.items():
    try:
        mol = indigo.loadIdt(idt_seq)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if text == error:
            print("Got expected error '%s'" % error)
        else:
            print("Expected error '%s' but got '%s'" % (error, text))
