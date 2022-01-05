import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()

for idx, r in enumerate(
    indigo.iterateRDFile(joinPathPy("../basic/reactions/rxns.rdf", __file__))
):
    print("*** %d ***" % (idx))
    try:
        print(r.smiles())
        print(r.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
