import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

mols = list(indigo.iterateSDFile(joinPathPy('molecules/aff.sdf', __file__)))

for m in mols:
    print(m.smiles())

options = [ "", "0.1", "0.5", "1.0", "10.0" ]

for opt in options:
    print("Exact options: " + opt)
    for m in mols:
        res = []
        for m2 in mols:
            match = indigo.exactMatch(m, m2, opt)
            if match:
                res.append("1")
            else:
                res.append("0")

        print(" ".join(res))
