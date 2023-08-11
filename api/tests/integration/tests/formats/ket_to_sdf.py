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
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to SDF ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "acd2d_err3",
]

files.sort()
for filename in files:
    try:
        ket = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    except:
        try:
            ket = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename + ".ket"))
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

    buffer = indigo.writeBuffer()
    sdfSaver = indigo.createSaver(buffer, "sdf")
    for frag in ket.iterateComponents():
        sdfSaver.append( frag.clone() )
    sdfSaver.close()
    sdf = buffer.toString()
    # with open(os.path.join(ref_path, filename) + ".sdf", "w") as file:
    #   file.write(sdf)

    with open(os.path.join(ref_path, filename) + ".sdf", "r") as file:
        sdf_ref = file.read()
    diff = find_diff(sdf_ref, sdf)
    if not diff:
        print(filename + ".sdf:SUCCEED")
    else:
        print(filename + ".sdf:FAILED")
        print(diff)
