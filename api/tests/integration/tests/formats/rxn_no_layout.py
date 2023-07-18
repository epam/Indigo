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

print("*** Rxn without layout to KET ***")

root = joinPathPy("reactions", __file__)
files = ["1113-no-layout.rxn "]
for filename in files:
    print(filename)
    rxn = indigo.loadReactionFromFile(os.path.join(root, filename))
    print(rxn.json())
