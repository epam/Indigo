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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to CML ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["1878-ket-to-cml"]

files.sort()
for filename in files:
    ket = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))

    buffer = indigo.writeBuffer()
    cmlSaver = indigo.createSaver(buffer, "cml")
    cmlSaver.close()
    cml = buffer.toString()

    with open(os.path.join(ref_path, filename) + ".cml", "r") as file:
        cml_ref = file.read()
    diff = find_diff(cml_ref, cml)
    if not diff:
        print(filename + ".cml:SUCCEED")
    else:
        print(filename + ".cml:FAILED")
        print(diff)
