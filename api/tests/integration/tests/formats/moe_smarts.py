#!/bin/env python3
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", "true")

input_path = joinPathPy("molecules", __file__)

with open(os.path.join(input_path, "moe.smarts"), "r", encoding="utf-8") as f1:
    moe_smarts = f1.read().splitlines()

with open(
    os.path.join(input_path, "crippen.smiles"), "r", encoding="utf-8"
) as f2:
    crippen_smiles = f2.read().splitlines()

for smile in crippen_smiles:
    ideal_fingerprint = 0
    smile_mol = indigo.loadMolecule(smile)
    for index, smart in enumerate(moe_smarts):
        moe_query = indigo.loadSmarts(smart)
        if indigo.substructureMatcher(smile_mol).match(moe_query):
            ideal_fingerprint = ideal_fingerprint + 2**index
    print("%s: %d" % (smile, ideal_fingerprint))
