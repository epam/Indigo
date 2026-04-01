import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
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
    seq = mol.fasta(lib)
    compare_diff(ref_path, filename + ".fasta", seq)
