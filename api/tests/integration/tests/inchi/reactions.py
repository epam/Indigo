#!/usr/bin/env python3

import os
import sys

# adjust this path as needed to import env_indigo
sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

# Initialize Indigo and InChI generator
indigo = Indigo()
indigo.setOption("ignore-noncritical-query-features", "true")
indigo_inchi = IndigoInchi(indigo)

# List of SMILES to test
input_smiles = [
    "CCN([C@H]1CO[C@H](C[C@@H]1OC)O[C@@H]2[C@H]([C@@H]([C@H](O[C@H]2O[C@H]3C#C/C=C\\C#C[C@@]\\4(CC(=O)C(=C3/C4=C\\CSSC(C)(C)CC(=O)N/N=C(/C)\\C5=CC=C(C=C5)OCCCC(=O)N)NC(=O)OC)O)C)NO[C@H]6C[C@@H]([C@@H]([C@H](O6)C)SC(=O)C7=C(C(=C(C(=C7OC)OC)O[C@H]8[C@@H]([C@@H]([C@H]([C@@H](O8)C)O)OC)O)I)C)O)O)C(=O)C",
    "OC[C@@](C(=O)O[C@H]1CN2CCC1CC2)(C2=CC=CC=C2)CSC |o1:2|",
    "CO[C@H]1C[C@H](O[C@H]2[C@H](C)O[C@@H](O[C@@H]3/C(C)=C/C[C@@H]4C[C@@H](C[C@]5(C=C[C@H](C)[C@@H](C6CCCCC6)O5)O4)OC(=O)[C@@H]4C=C(C)[C@@H](O)[C@H]5OC/C(=C\\C=C\\[C@@H]3C)[C@@]45O)C[C@@H]2OC)O[C@@H](C)[C@@H]1O",
]


def load_molecule(data):
    """
    Load a molecule from SMILES or other raw data,
    retrying with relaxed stereochemistry errors if needed.
    """
    # first, enforce strict stereochemistry
    indigo.setOption("ignore-stereochemistry-errors", "0")
    try:
        return indigo.loadMolecule(data)
    except IndigoException as e:
        # on failure, print the error and retry more permissively
        print("  Exception:", getIndigoExceptionText(e))
        indigo.setOption("ignore-stereochemistry-errors", "1")
        return indigo.loadMolecule(data)


# Iterate over each SMILES, generate InChI before and after reaction reload
for idx, smiles in enumerate(input_smiles, start=1):
    m = load_molecule(smiles)
    try:
        # generate the "expected" InChI
        inchi1 = indigo_inchi.getInchi(m)
        print("  Smiles original:", smiles)
        print("  InChI1:", inchi1)

        # create a one-reactant reaction, then reload it (lossy)
        rxn = indigo.createReaction()
        rxn.addReactant(m)
        rxn = indigo.loadReaction(rxn.smiles())

        # extract the reactant and get its InChI
        reactant = next(rxn.iterateReactants())
        inchi2 = indigo_inchi.getInchi(reactant)
        print("  InChI2:", inchi2)

        # compare
        if inchi1 != inchi2:
            print("  -> Mismatch detected!")
        else:
            print("  -> InChIs match.")
        print()

    except IndigoException as e:
        # catch any Indigo errors during processing
        print("  Error:", getIndigoExceptionText(e))
        warn = indigo_inchi.getWarning()
        log = indigo_inchi.getLog()
        if warn:
            print("  InChI warning:", warn)
        if log:
            print("  InChI log:", log)
        print()
