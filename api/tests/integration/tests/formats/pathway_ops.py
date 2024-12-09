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
input_path = joinPathPy("reactions/rdf/", __file__)
ref_path_arom = joinPathPy("ref/arom/", __file__)
ref_path_dearom = joinPathPy("ref/dearom/", __file__)

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
]

print("*** PATHWAY AROMATIZE/DEAROMATIZE ***")

for filename in files:
    rxn = indigo.loadReactionFromFile(
        os.path.join(input_path, filename + ".rdf")
    )

    # aromatize check
    rxn.aromatize()
    rxn_txt = rxn.json()
    # with open(os.path.join(ref_path_arom, filename) + ".ket", "w") as file:
    #     file.write(rxn_txt)

    rxn_ref = open(os.path.join(ref_path_arom, filename) + ".ket", "r").read()
    diff_arom = find_diff(rxn_ref, rxn_txt)

    # dearomatize check
    rxn.dearomatize()
    rxn_txt = rxn.json()
    # with open(os.path.join(ref_path_dearom, filename) + ".ket", "w") as file:
    #     file.write(rxn_txt)

    rxn_ref = open(
        os.path.join(ref_path_dearom, filename) + ".ket", "r"
    ).read()
    diff_dearom = find_diff(rxn_ref, rxn_txt)

    diff = diff_arom and diff_dearom

    if not diff:
        print(filename + ".rdf:SUCCEED")
    else:
        print(filename + ".rdf:FAILED")
        print(diff)
