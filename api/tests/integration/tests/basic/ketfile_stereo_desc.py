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

ref_path = joinPathPy("ref/", __file__)
root = joinPathPy("molecules/CIP/", __file__)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-saving-add-stereo-desc", True)

for filename in sorted(os.listdir(root)):
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    except Exception as e:
        print(e)
        continue
    ketfile = joinPathPy(
        os.path.join(ref_path, filename[:-4] + ".ket"), __file__
    )
    with open(ketfile, "r") as file:
        ket_ref = file.read()
        try:
            diff = ket_ref != mol.json()
        except:
            print(e)
            diff = "exception"
        if not diff:
            print(filename + ":SUCCEED")
        else:
            print(filename + ":FAILED")
            print(diff)

indigo.setOption("ignore-stereochemistry-errors", "true")
filename = "crazystereo.rxn"

rxn = indigo.loadReactionFromFile(
    os.path.join(joinPathPy("reactions/", __file__), filename)
)

ketfile = joinPathPy(os.path.join(ref_path, "crazystereo.ket"), __file__)
with open(ketfile, "r") as file:
    ket_ref = file.read()
    diff = ket_ref != rxn.json()
    if not diff:
        print(filename + ":SUCCEED")
    else:
        print(filename + ":FAILED")
        print(diff)
