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
input_path = joinPathPy("reactions/rdf", __file__)
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

for filename in files:
    rxn = indigo.loadReactionFromFile(
        os.path.join(input_path, filename + ".rdf")
    )

    rxn_txt = rxn.json()

    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(rxn_txt)

    rxn_ref = open(getRefFilepath(filename + ".ket"), "r").read()

    diff = find_diff(rxn_ref, rxn_txt)
    if not diff:
        print(filename + ".rdf:SUCCEED")
    else:
        print(filename + ".rdf:FAILED")
        print(diff)
