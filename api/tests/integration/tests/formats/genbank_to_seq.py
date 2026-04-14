import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** GenBank/GenPept to Seq***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    {"file": "1844-gen_bank", "seq_type": "PEPTIDE"},
    {"file": "1844-gen_pept", "seq_type": "PEPTIDE"},
    {"file": "2763-gen_spaces", "seq_type": "DNA"},
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for infile in files:
    filename = infile["file"] + ".seq"
    mol = indigo.loadSequenceFromFile(
        os.path.join(root, filename), infile["seq_type"], lib
    )
    seq = mol.sequence(lib)
    compare_diff(ref_path, filename, seq)
