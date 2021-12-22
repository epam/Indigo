import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("timeout", "2000")

t = indigo.loadMolecule(
    "N(C1N=CC=C2C=1C=C(C1C=CC=CC=1)C(C1C=CC(C3(NC(=O)OC(C)(C)C)CC4(OCCO4)C3)=CC=1)=N2)N"
)
q = indigo.loadQueryMolecule(
    "C.C.C.C.C.O.O.CC1(OCCO1)C.C=C(C1C(C2C=CC=CC=2)=CC2C3=NN=C(C4N=CN(C)C=4)N3C=CC=2N=1)C"
)


def test(t, q):
    matcher = indigo.substructureMatcher(t)
    try:
        print(matcher.match(q) != None)
    except IndigoException as e:
        print(getIndigoExceptionText(e))


# test multiple times
test(t, q)
test(t, q)
test(t, q)

t = indigo.loadMolecule("C1N=CC=CC=1C=C(C1C=CC=CC=1)")
q = indigo.loadQueryMolecule("C.C.C.C.C.O")
test(t, q)
test(t, q)
