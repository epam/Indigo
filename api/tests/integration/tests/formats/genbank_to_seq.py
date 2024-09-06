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

print("*** GenBank/GenPept to Seq***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    {"file": "1844-gen_bank", "seq_type": "PEPTIDE"},
    {"file": "1844-gen_pept", "seq_type": "PEPTIDE"},
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for infile in files:
    filename = infile["file"] + ".seq"
    mol = indigo.loadSequenceFromFile(
        os.path.join(root, filename), infile["seq_type"], lib
    )
    with open(os.path.join(ref_path, filename), "r") as file:
        seq_ref = file.read()
    seq = mol.sequence(lib)
    diff = find_diff(seq_ref, seq)
    if not diff:
        print(filename + ".seq:SUCCEED")
    else:
        print(filename + ".seq:FAILED")
        print(diff)
