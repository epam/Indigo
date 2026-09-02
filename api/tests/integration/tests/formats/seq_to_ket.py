import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import (
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

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
    {"seq_type": "PEPTIDE", "seq_data": "BJZX", "ref": "aminoacids_variants"},
    {"seq_type": "RNA", "seq_data": "RKN", "ref": "rna_variants"},
    {"seq_type": "DNA", "seq_data": "BN", "ref": "dna_variants"},
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for seq in seq_tests:
    mol = indigo.loadSequence(seq["seq_data"], seq["seq_type"], lib)
    filename = seq["ref"]
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)

seq_errors = {
    "12w12r23e32e33": (
        "PEPTIDE",
        "Invalid symbols in the sequence: 1,2,2,3,3,2,3,3",
    ),
    "12w12r23c32c33": (
        "RNA",
        "Invalid symbols in the sequence: 1,2,2,3,3,2,3,3",
    ),
}
for seq in sorted(seq_errors.keys()):
    type, error = seq_errors[seq]
    try:
        mol = indigo.loadSequence(seq, type, lib)
        print("Test %s %s failed: exception expected." % (type, seq))
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test %s '%s': got expected error '%s'" % (type, seq, error))
        else:
            print(
                "Test %s '%s': expected error '%s' but got '%s'"
                % (type, seq, error, text)
            )
