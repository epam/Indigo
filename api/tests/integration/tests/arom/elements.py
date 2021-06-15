import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

def testFile(filename, shortname):
    print("%s" % shortname)
    idx = 0
    for m in indigo.iterateSDFile(filename):
        print(idx)
        try:
            print(m.canonicalSmiles())
            m.aromatize()
            print(m.canonicalSmiles())
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))
        idx += 1

for root, dirnames, filenames in os.walk(joinPath("molecules/elements")):
    for filename in sorted(filenames):
        testFile(os.path.join(root, filename), filename)
