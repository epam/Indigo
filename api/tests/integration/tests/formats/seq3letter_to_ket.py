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
    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(doc.json())
    with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
        ket_ref = file.read()
    ket = doc.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + " : SUCCEED")
    else:
        print(filename + " : FAILED")
        print(diff)

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
