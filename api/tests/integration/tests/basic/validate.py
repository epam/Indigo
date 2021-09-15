# Test for checkBadValence and checkAmbiguousH methods

import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

q = indigo.loadSmarts("[N,H3]")

def doTest (m):
    try:
        ret = indigo.substructureMatcher(m).match(q)
        print(ret is None)
    except IndigoException as e:
        print("Exception: %s" % (getIndigoExceptionText(e)))

for idx, m in enumerate(indigo.iterateSDFile(joinPathPy("molecules/bad_valence.sdf", __file__))):
    print("** %d **" % idx)
    doTest(m)
    print(m.checkBadValence())
    doTest(m)
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print("Exception: %s" % (getIndigoExceptionText(e)))
    doTest(m)
