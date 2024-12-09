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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** RDF to CDXML ***")

root = joinPathPy("reactions/rdf/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "pathway1",
    "pathway2",
    "pathway3",
    "pathway4",
    "pathway5",
    "pathway6",
    "pathway7",
    "pathway8",
    "pathway9",
    "pathway10",
    "pathway11",
    "pathway12",
]

files.sort()
for filename in files:
    try:
        ket = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".rdf")
        )
    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))

    cdxml_text = ket.cdxml()
    # with open(os.path.join(ref_path, filename + ".cdxml"), "w") as file:
    #     file.write(cdxml_text)

    with open(os.path.join(ref_path, filename) + ".cdxml", "r") as file:
        cdxml_ref = file.read()

    diff = find_diff(cdxml_ref, cdxml_text)
    if not diff:
        print(filename + ".cdxml:SUCCEED")
    else:
        print(filename + ".cdxml:FAILED")
        print(diff)
