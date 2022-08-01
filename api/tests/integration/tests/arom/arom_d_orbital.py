import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
for idx, m in enumerate(
    indigo.iterateSDFile(joinPathPy("molecules/mols.sdf", __file__))
):
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
