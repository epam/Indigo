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

print("*** KET to KET ***")

# root = joinPathPy("molecules/", __file__)
# ref_path = joinPathPy("ref/", __file__)
root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["super_atom_attachment_points"]

files.sort()
for filename in files:
    ket_in = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    # print(ket_in.json())
    # with open(os.path.join(ref_path, filename) + "_out"+".ket", "w") as file:
    #    file.write(ket_in.json())
    with open(os.path.join(ref_path, filename) + "_out" + ".ket", "r") as file:
        ket_ref = file.read()
    ket = ket_in.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
       print(filename + ".ket:SUCCEED")
    else:
       print(filename + ".ket:FAILED")
       print(diff)
