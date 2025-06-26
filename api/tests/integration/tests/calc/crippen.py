import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

try:
    from collections.abc import Iterable
except ImportError:
    from collections import Iterable

try:
    basestring
except NameError:
    basestring = str


def check_float(method, smiles, expected, delta=1e-2):
    m = indigo.loadMolecule(smiles)
    actual = getattr(m, method)()

    def _c(a, e, p=""):
        if (
            isinstance(a, Iterable)
            and isinstance(e, Iterable)
            and not isinstance(a, basestring)
        ):
            if len(a) != len(e):
                print(
                    "%s %s%s: len(actual)=%d != len(expected)=%d"
                    % (m, method, p, len(a), len(e))
                )
            for i, (ai, ei) in enumerate(zip(a, e)):
                _c(ai, ei, "%s[%d]" % (p, i))
        else:
            try:
                if abs(a - e) > delta:
                    print("%s %s%s: %r != %r" % (m, method, p, a, e))
            except:
                print(
                    "%s %s%s: cannot compare %r and %r" % (m, method, p, a, e)
                )

    _c(actual, expected)


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


def test_multi_pka():
    check_float("pKaValues", "PEPTIDE:A", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:R", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:N", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:D", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:C", [2.39, 8.493334, 9.530001])
    check_float("pKaValues", "PEPTIDE:Q", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:E", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:G", [2.9135, 9.605])
    check_float("pKaValues", "PEPTIDE:H", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:I", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:L", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:K", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:M", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:F", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:P", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:S", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:T", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:W", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:Y", [2.39, 9.530001])
    check_float("pKaValues", "PEPTIDE:V", [2.39, 9.530001])


if __name__ == "__main__":
    indigo = Indigo()
    test_logp()
    test_mr()
    test_multi_pka()
