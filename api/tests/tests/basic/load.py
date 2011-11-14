import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
for root, dirnames, filenames in os.walk(joinPath("molecules/set1")):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s: " % filename)
        try:
            mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
            print("   OK")
        except IndigoException, e:
            print("  %s" % (getIndigoExceptionText(e)))
