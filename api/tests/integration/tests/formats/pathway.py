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
    "pathway_mon",
]

for filename in files:
    rxn = indigo.loadReactionFromFile(
        os.path.join(input_path, filename + ".rdf")
    )
    rxn_txt = rxn.json()
    compare_diff(ref_path, filename + ".ket", rxn_txt)
