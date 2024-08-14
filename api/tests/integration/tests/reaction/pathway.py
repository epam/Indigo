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
from env_indigo import *

indigo = Indigo()
input_path = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["pathway1", "pathway2", "pathway3", "pathway4", "pathway5"]

for filename in files:
    rxn = indigo.loadReactionFromFile(
        os.path.join(input_path, filename + ".rdf")
    )
    rxn_ref = open(os.path.join(ref_path, filename) + ".ket", "r").read()

    rxn_txt = rxn.json()

    diff = find_diff(rxn_ref, rxn_txt)
    if not diff:
        print(filename + ".rdf:SUCCEED")
    else:
        print(filename + ".rdf:FAILED")
        print(diff)
