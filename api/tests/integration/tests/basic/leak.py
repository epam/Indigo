import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("**** Potential leak on exception in SDF loading ****")
# Check open file handles leak in case of exception
for i in range(100):
    try:
        all = indigo.createMolecule()
        for m in indigo.iterateSDFile(joinPath("molecules/exceptions/empty.sdf")):
            all.merge(m)
    except IndigoException as ex:
        pass
        
    m = indigo.loadMoleculeFromFile(joinPath("molecules/exceptions/nonempty.mol"))
    print("%d: %s" % (i, m.smiles()))
