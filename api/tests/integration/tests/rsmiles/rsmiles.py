import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

idx = 1
for item in indigo.iterateSDFile(joinPathPy("reactions/sch_50.sdf", __file__)):
    try:
        print("*** %d ***" % (idx))
        idx += 1
        r = indigo.loadReaction(item.rawData())
        print(r.smiles())
        print(r.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
