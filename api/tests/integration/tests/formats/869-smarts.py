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
indigo.setOption("ignore-bad-valence", True)

mol = indigo.loadMolecule("OC(=O)c1ccc(OCC2=CC=CC=C2)cc1")
indigo.setOption("smiles-loading-strict-aliphatic", False)
q1_str = "[OH][i](=O)[i]~[i]~[i]~[i]~[i]-A"
q2_str = "[O][i]~[i]~[i]~[i]~[i]~[i]~[i]~A"
q3_str = "[OH][i]~[i](~*)~*"
query1 = indigo.loadSmarts(q1_str)
query2 = indigo.loadSmarts(q2_str)
query3 = indigo.loadSmarts(q3_str)

if indigo.substructureMatcher(mol).match(query1):
    print("4-(benzyloxy)benzoic acid matches with " + q1_str)

if indigo.substructureMatcher(mol).match(query2):
    print("4-(benzyloxy)benzoic acid matches with " + q2_str)

if indigo.substructureMatcher(mol).match(query3):
    print("4-(benzyloxy)benzoic acid matches with " + q3_str)

indigo.setOption("smiles-loading-strict-aliphatic", True)
query1 = indigo.loadSmarts(q1_str)
query2 = indigo.loadSmarts(q2_str)
query3 = indigo.loadSmarts(q3_str)

if not indigo.substructureMatcher(mol).match(query1):
    print(
        "4-(benzyloxy)benzoic acid doesn't match with [OH][i](=O)[i]~[i]~[i]~[i]~[i]-A"
    )

if not indigo.substructureMatcher(mol).match(query2):
    print(
        "4-(benzyloxy)benzoic acid doesn't match with [O][i]~[i]~[i]~[i]~[i]~[i]~[i]~A"
    )

if indigo.substructureMatcher(mol).match(query3):
    print("4-(benzyloxy)benzoic acid matches with " + q3_str)
