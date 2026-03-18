import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
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
    rxn_txt_arom = rxn.json()
    diff_arom = compare_diff(
        ref_path_arom, filename + ".ket", rxn_txt_arom, stdout=False
    )

    # dearomatize check
    rxn.dearomatize()
    rxn_txt_dearom = rxn.json()
    diff_dearom = compare_diff(
        ref_path_dearom, filename + ".ket", rxn_txt_dearom, stdout=False
    )

    diff = diff_arom and diff_dearom

    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
