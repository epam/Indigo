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

print("*** KET with query properties ***")

ref_path = joinPathPy("ref/", __file__)

filename = os.path.join(ref_path, "ket_with_query_properties.ket")

mol = indigo.loadQueryMoleculeFromFile(filename)
with open(filename, "r") as file:
    ket_ref = file.read()
ket = mol.json()
diff = find_diff(ket_ref, ket)
if not diff:
    print(filename + ".ket:SUCCEED")
else:
    print(filename + ".ket:FAILED")
    print(diff)
