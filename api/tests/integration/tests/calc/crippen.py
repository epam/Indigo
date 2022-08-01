import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa


def check_float(method, smiles, expected, delta=1e-2):
    global indigo
    m = indigo.loadMolecule(smiles)
    actual = getattr(m, method)()
    if abs(actual - expected) > delta:
        print(m, method, actual, "!=", expected)


def test_logp():
    check_float("logP", "c1ccccc1", 1.68)
    check_float("logP", "C[U]", 0.5838)
    check_float("logP", "CSc1ccc2Sc3ccccc3N(CCC4CCCCN4C)c2c1", 5.8856)
    check_float("logP", "Nc1ccccc1", 1.2688)
    check_float("logP", "CN1C=NC2=C1C(=O)N(C(=O)N2C)C", 0.06)
    check_float(
        "logP",
        "C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O",
        3.7,
    )


def test_mr():
    check_float("molarRefractivity", "c1ccccc1", 26.442)
    check_float("molarRefractivity", "C[U]", 5.86)
    check_float("molarRefractivity", "Clc1ccccc1", 31.45)


def test_pka():
    check_float("pKa", "Cc1cc(O)cc(c1)[N+](C)(C)C", 8.1999998)
    check_float("pKa", "Cc1ccc(cc1)C1C[NH2+]1", 9.53)
    check_float("pKa", "O=C(NBr)C(Cl)Cl", 4.1549997)


if __name__ == "__main__":
    indigo = Indigo()
    test_logp()
    test_mr()
