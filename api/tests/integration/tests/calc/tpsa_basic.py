import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()


def checkTPSA(smiles, expected, includeSP=False, eps=0.1):
    if includeSP:
        actual = indigo.loadMolecule(smiles).tpsa(includeSP)
    else:
        actual = indigo.loadMolecule(smiles).tpsa()
    if abs(actual - expected) > eps:
        raise ValueError("Incorrect TPSA for SMILES {}: {} (should be {})".format(smiles, actual, expected))


checkTPSA("C", 0.0)
checkTPSA("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 58.44)
checkTPSA("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", 141.31)
checkTPSA("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", 149.69, True)
