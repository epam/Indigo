import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
root_rxn = joinPathPy("reactions/", __file__)
filename = "1113-no-layout.rxn"
print(filename)
rxn = indigo.loadReactionFromFile(os.path.join(root_rxn, filename))
print(rxn.json())
