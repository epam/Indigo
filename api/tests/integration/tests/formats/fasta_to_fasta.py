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

print("*** FASTA to FASTA ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

fasta_files = [
    {"file": "test_peptide", "seq_type": "PEPTIDE"},
    {"file": "test_rna", "seq_type": "RNA"},
    {"file": "test_dna", "seq_type": "DNA"},
    {"file": "multiseq", "seq_type": "DNA"},
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for desc in fasta_files:
    filename = desc["file"]
    mol = indigo.loadFastaFromFile(
        os.path.join(root, filename + ".fasta"), desc["seq_type"], lib
    )
    # with open(os.path.join(ref_path, filename) + ".fasta", "w") as file:
    #     file.write(mol.fasta())
    with open(os.path.join(ref_path, filename) + ".fasta", "r") as file:
        fasta_ref = file.read()
    fasta = mol.fasta(lib)
    diff = find_diff(fasta_ref, fasta)
    if not diff:
        print(filename + ".fasta:SUCCEED")
    else:
        print(filename + ".fasta:FAILED")
        print(diff)
