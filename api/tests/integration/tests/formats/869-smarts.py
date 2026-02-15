#!/bin/env python3
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa: F401
    Bingo,
    BingoException,
    BingoObject,
    Indigo,
    IndigoException,
    IndigoInchi,
    IndigoObject,
    IndigoRenderer,
    dataPath,
    dir_exists,
    file_exists,
    file_size,
    getIndigoExceptionText,
    joinPathPy,
    makedirs,
    relativePath,
    rmdir,
    threading,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)


def check_smarts(mol, smarts):
    query = indigo.loadSmarts(smarts)
    m = (
        "matches"
        if indigo.substructureMatcher(mol).match(query)
        else "doesn't match"
    )
    print("4-(benzyloxy)benzoic acid %s with %s" % (m, smarts))


mol = indigo.loadMolecule("OC(=O)c1ccc(OCC2=CC=CC=C2)cc1")
indigo.setOption("smiles-loading-strict-aliphatic", False)
q1_str = "[OH][i](=O)[i]~[i]~[i]~[i]~[i]-A"
q2_str = "[O][i]~[i]~[i]~[i]~[i]~[i]~[i]~A"
q3_str = "[OH][i]~[i](~*)~*"

check_smarts(mol, q1_str)
check_smarts(mol, q2_str)
check_smarts(mol, q3_str)

indigo.setOption("smiles-loading-strict-aliphatic", True)

check_smarts(mol, q1_str)
check_smarts(mol, q2_str)
check_smarts(mol, q3_str)
