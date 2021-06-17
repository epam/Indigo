import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
for idx, m in enumerate(indigo.iterateSDFile(joinPath('molecules', 'mols.sdf'))):
    print("*** %d ***" % (idx))
    try:
        print(m.smiles())
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        m.dearomatize()
        print(m.smiles())
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
