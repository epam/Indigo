# Test for checkBadValence and checkAmbiguousH methods

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

q = indigo.loadSmarts("[N,!#1&H3]")


def doTest(m):
    try:
        ret = indigo.substructureMatcher(m).match(q)
        print(ret is None)
    except IndigoException as e:
        print("Exception: %s" % (getIndigoExceptionText(e)))


for idx, m in enumerate(
    indigo.iterateSDFile(joinPathPy("molecules/bad_valence.sdf", __file__))
):
    print("** %d **" % idx)
    doTest(m)
    print(m.checkBadValence())
    doTest(m)
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print("Exception: %s" % (getIndigoExceptionText(e)))
    doTest(m)
