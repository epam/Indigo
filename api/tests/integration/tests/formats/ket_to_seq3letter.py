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
    with open(os.path.join(refp, filename) + ".seq3", "w") as file:
        file.write(doc.sequence3Letter(lib))
    with open(os.path.join(refp, filename) + ".seq3", "r") as file:
        seq_ref = file.read()
    seq = doc.sequence3Letter(lib)
    diff = find_diff(seq_ref, seq)
    if not diff:
        print(filename + " : SUCCEED")
    else:
        print(filename + " : FAILED")
        print(diff)
