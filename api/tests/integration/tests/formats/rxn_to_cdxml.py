import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** Rxn to CDXML ***")

root = joinPathPy("reactions/cdxml", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        print("*** Try as Reaction ***")
        rxn = indigo.loadReactionFromFile(os.path.join(root, filename))
        indigo.setOption("layout-horintervalfactor", "1.4")
        print(rxn.cdxml())
        print("*** Compare with QueryReaction ***")
        rxn2 = indigo.loadQueryReactionFromFile(os.path.join(root, filename))
        print(rxn2.cdxml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as QueryReaction ***")
        rxn = indigo.loadQueryReactionFromFile(os.path.join(root, filename))
        indigo.setOption("layout-horintervalfactor", "1.4")
        print(rxn.cdxml())
