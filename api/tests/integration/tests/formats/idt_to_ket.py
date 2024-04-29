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
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** IDT to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

idt_names = ["idt_acg", "idt_maxmgc", "idt_2moera", "idt_modifications"]

idt_data = {
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
