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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to IDT ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "1654-dna-to-idt",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    # with open(os.path.join(ref_path, filename) + ".idt", "w") as file:
    #     file.write(mol.idt())
    with open(os.path.join(ref_path, filename) + ".idt", "r") as file:
        idt_ref = file.read()
    idt = mol.idt()
    diff = find_diff(idt_ref, idt)
    if not diff:
        print(filename + ".idt:SUCCEED")
    else:
        print(filename + ".idt:FAILED")
        print(diff)
