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

print("*** SEQUENCE to KET ***")

ref_path = joinPathPy("ref/", __file__)

seq_tests = [
    {
        "seq_type": "PEPTIDE",
        "seq_data": "ACDEFGHIKLMNOPQRSRUVWY",
        "ref": "all_aminoacids",
    },
    {"seq_type": "RNA", "seq_data": "ACGTU", "ref": "rna_acgtu"},
    {"seq_type": "DNA", "seq_data": "ACGTU", "ref": "dna_acgtu"},
    {
        "seq_type": "PEPTIDE",
        "seq_data": "ACD\nEFG\r\nHIKLMN OPQRSRUVWY",
        "ref": "spaces",
    },
]

for seq in seq_tests:
    mol = indigo.loadSequence(seq["seq_data"], seq["seq_type"])
    filename = seq["ref"]
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
