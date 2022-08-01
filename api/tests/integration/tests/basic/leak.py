import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("**** Potential leak on exception in SDF loading ****")
# Check open file handles leak in case of exception
for i in range(100):
    try:
        all = indigo.createMolecule()
        for m in indigo.iterateSDFile(
            joinPathPy("molecules/exceptions/empty.sdf", __file__)
        ):
            all.merge(m)
    except IndigoException as ex:
        pass

    m = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/exceptions/nonempty.mol", __file__)
    )
    print("%d: %s" % (i, m.smiles()))
