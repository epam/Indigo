import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

for idx, r in enumerate(
    indigo.iterateSDFile(joinPathPy("molecules/e-z-isomerism.sdf", __file__))
):
    print("*** %d ***" % (idx))
    try:
        print(r.smiles())
        print(r.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
