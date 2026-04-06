import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** 3 LETTER SEQUENCE to KET ***")

ref_path = joinPathPy("ref/", __file__)

files = [
    "peptides_3letter",
    "peptides_3letter_line_break",
]


lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library.ket")
)

for filename in files:
    doc = indigo.loadSequenceFromFile(
        os.path.join(ref_path, filename + ".seq3"), "PEPTIDE-3-LETTER", lib
    )
    ket = doc.json()
    compare_diff(ref_path, filename + ".ket", ket)

seq3_errors = {
    "ala": "Given string cannot be interpreted as a valid three letter sequence because of incorrect formatting.",
    "ALA": "Given string cannot be interpreted as a valid three letter sequence because of incorrect formatting.",
    "Al a": "Given string cannot be interpreted as a valid three letter sequence because of incorrect formatting.",
}
for seq3 in sorted(seq3_errors.keys()):
    error = seq3_errors[seq3]
    try:
        mol = indigo.loadSequence(seq3, "PEPTIDE-3-LETTER", lib)
        print("Test %s failed: exception expected." % seq3)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (seq3, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (seq3, error, text)
            )
