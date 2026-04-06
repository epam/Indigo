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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to 3 LETTER SEQUENCE ***")

refp = joinPathPy("ref/", __file__)

files = [
    "peptides_3letter",
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(refp, "monomer_library.ket")
)

files.sort()
for filename in files:
    doc = indigo.loadKetDocumentFromFile(os.path.join(refp, filename + ".ket"))
    seq = doc.sequence3Letter(lib)
    compare_diff(refp, filename + ".seq3", seq)

seq_errors = {
    "peptides_molecule": "Sequence saver: Can't save micro-molecules to sequence format",
    "issue_3200": "Only amino acids can be saved as three letter amino acid codes",
}

for filename in sorted(seq_errors.keys()):
    error = seq_errors[filename]
    doc = indigo.loadKetDocumentFromFile(os.path.join(refp, filename + ".ket"))
    try:
        seq = doc.sequence3Letter(lib)
        print("Test %s failed: exception expected." % filename)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test '%s': got expected error '%s'" % (filename, error))
        else:
            print(
                "Test '%s': expected error '%s' but got '%s'"
                % (filename, error, text)
            )
