import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def checkNumRotatableBonds(smiles, expected):
    actual = indigo.loadMolecule(smiles).numRotatableBonds()
    if actual != expected:
        print(
            "Incorrect numRotatableBonds for SMILES {}: {} (should be {})".format(
                smiles, actual, expected
            )
        )


def checkNumHydrogenBondAcceptors(smiles, expected):
    actual = indigo.loadMolecule(smiles).numHydrogenBondAcceptors()
    if actual != expected:
        print(
            "Incorrect numHydrogenBondAcceptors for SMILES {}: {} (should be {})".format(
                smiles, actual, expected
            )
        )


def checkNumHydrogenBondDonors(smiles, expected):
    actual = indigo.loadMolecule(smiles).numHydrogenBondDonors()
    if actual != expected:
        print(
            "Incorrect checkNumHydrogenBondDonors for SMILES {}: {} (should be {})".format(
                smiles, actual, expected
            )
        )


checkNumRotatableBonds("C", 0)
checkNumRotatableBonds("CC", 0)
checkNumRotatableBonds("CCC", 0)
checkNumRotatableBonds("CCCC", 1)
checkNumRotatableBonds("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 0)
checkNumRotatableBonds(
    "C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", 6
)

checkNumHydrogenBondAcceptors("C", 0)
checkNumHydrogenBondAcceptors("CC", 0)
checkNumHydrogenBondAcceptors("CCC", 0)
checkNumHydrogenBondAcceptors("CCCC", 0)
checkNumHydrogenBondAcceptors("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 6)
checkNumHydrogenBondAcceptors(
    "C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", 9
)

checkNumHydrogenBondDonors("C", 0)
checkNumHydrogenBondDonors("CC", 0)
checkNumHydrogenBondDonors("CCC", 0)
checkNumHydrogenBondDonors("CCCC", 0)
checkNumHydrogenBondDonors("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 0)
checkNumHydrogenBondDonors(
    "C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", 3
)
