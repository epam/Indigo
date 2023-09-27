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

print("*** KET with query components ***")

ref_path = joinPathPy("ref/", __file__)
name = "ket_with_query_components.ket"
filename = os.path.join(ref_path, name)

mol = indigo.loadQueryMoleculeFromFile(filename)
with open(filename, "r") as file:
    ket_ref = file.read()
ket = mol.json()
diff = find_diff(ket_ref, ket)
if not diff:
    print(name + ":SUCCEED")
else:
    print(name + ":FAILED")
    print(diff)
