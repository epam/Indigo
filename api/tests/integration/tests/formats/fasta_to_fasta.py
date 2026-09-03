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
    try:
        mol = indigo.loadFastaFromFile(
            os.path.join(root, filename + ".fasta"), desc["seq_type"], lib
        )
    except Exception as e:
        print("%s.fasta:FAILED - %s" % (filename, e))
        continue
    try:
        fasta = mol.fasta(lib)
    except Exception as e:
        print(filename + ".fasta:FAILED")
        print(e)
        continue
    compare_diff(ref_path, filename + ".fasta", fasta)
