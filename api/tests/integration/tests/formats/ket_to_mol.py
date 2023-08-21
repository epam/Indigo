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
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to MOL ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

files = ["suplabel"]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        ket_ref = file.read()
    ket = mol.molfile()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)

files = ["ket-reaction-arrow", "empty_apid"]

files.sort()
for filename in files:
    rc = indigo.loadReactionFromFile(os.path.join(root_rea, filename + ".ket"))
    ket = rc.rxnfile()
    # with open(os.path.join(ref_path, filename) + ".mol", "w") as file:
    #    file.write(ket)

    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        ket_ref = file.read()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
