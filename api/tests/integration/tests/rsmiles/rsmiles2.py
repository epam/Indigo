import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

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
