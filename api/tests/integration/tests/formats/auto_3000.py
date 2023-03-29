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
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("molfile-saving-mode", "auto")
indigo.setOption("ignore-stereochemistry-errors", True)

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["cis_trans","enhanced_stereo3", "atoms (or bonds) exceeds 999"]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    # with open(os.path.join(ref_path, filename) + ".mol", "w") as file:
    #    file.write(mol.molfile())
    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        mol_ref = file.read()

    mol_txt = mol.molfile()
    diff = find_diff(mol_ref, mol_txt)
    if not diff:
        print(filename + ".mol:SUCCEED")
    else:
        print(filename + ".mol:FAILED")
        print(diff)
