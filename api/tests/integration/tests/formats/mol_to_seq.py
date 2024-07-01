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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** MOL to SEQ ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "rna_mod",
    "conjugate",
    "dna_mod",
    "pepchem",
    "peptides",
    "fmoc",
    "anacyclamide",
    "acgt_1412",
    "apamine",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    # with open(os.path.join(ref_path, filename) + ".seq", "w") as file:
    #     file.write(mol.sequence())
    with open(os.path.join(ref_path, filename) + ".seq", "r") as file:
        seq_ref = file.read()
    seq = mol.sequence()
    diff = find_diff(seq_ref, seq)
    if not diff:
        print(filename + ".seq:SUCCEED")
    else:
        print(filename + ".seq:FAILED")
        print(diff)
