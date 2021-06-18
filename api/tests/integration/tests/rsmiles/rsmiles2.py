import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

for idx, r in enumerate(indigo.iterateRDFile(joinPath('..', 'basic', 'reactions', 'rxns.rdf'))):
    print("*** %d ***" % (idx))
    try:
        print(r.smiles())
        print(r.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
