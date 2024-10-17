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

print("*** KET to FASTA ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "1822-peptide",
    "1843-rna",
    "1950-mixed-seq",
    "nucleotides",
    "2341-no-analog",
    "2436-ambiguous",
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

files.sort()
for filename in files:
    mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
    # with open(os.path.join(ref_path, filename) + ".fasta", "w") as file:
    #     file.write(mol.fasta(lib))
    with open(os.path.join(ref_path, filename) + ".fasta", "r") as file:
        seq_ref = file.read()
    seq = mol.fasta(lib)
    diff = find_diff(seq_ref, seq)
    if not diff:
        print(filename + ".fasta:SUCCEED")
    else:
        print(filename + ".fasta:FAILED")
        print(diff)
