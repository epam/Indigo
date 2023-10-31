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
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** MOL to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "1032-quadra",
    "1046-imp_hydrogen",
    "SgroupDifferent",
    "suplabel",
    "atropisomer",
    "non_atrop",
    "cysteine",
    "dcysteine",
    "thymine",
    "dthymine",
    "chem",
    "rna_mod",
    "conjugate",
    "dna_mod",
    "pepchem",
    "fmoc"
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))

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
