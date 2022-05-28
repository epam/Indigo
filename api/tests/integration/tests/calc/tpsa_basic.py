import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()


def checkTPSA(smiles, expected, eps = 0.1):
    actual = indigo.loadMolecule(smiles).tpsa()
    if abs(actual - expected) > eps:
        raise ValueError("Incorrect TPSA for SMILES {}: {} (should be {})".format(smiles, actual, expected))


checkTPSA("C", 0.0)
checkTPSA("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 56.22)
