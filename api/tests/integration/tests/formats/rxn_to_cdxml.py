import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("*** Rxn to CDXML ***")

root = joinPath("reactions/cdxml")
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        print("*** Try as Reaction ***")
        rxn = indigo.loadReactionFromFile(joinPath(root, filename))
        indigo.setOption("layout-horintervalfactor", "1.4")
        rxn.layout()
        print(rxn.cdxml())
        print("*** Compare with QueryReaction ***")
        rxn2 = indigo.loadQueryReactionFromFile(joinPath(root, filename))
        rxn2.layout()
        print(rxn2.cdxml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as QueryReaction ***")
        rxn = indigo.loadQueryReactionFromFile(joinPath(root, filename))
        indigo.setOption("layout-horintervalfactor", "1.4")
        rxn.layout()
        print(rxn.cdxml())


