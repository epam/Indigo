import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo_inchi = IndigoInchi(indigo)

molit = indigo.iterateSDFile(joinPath("../../../../../data/molecules/basic/zinc-slice.sdf.gz"))
inchiit = open(joinPath("molecules/zinc-slice.sdf.inchi")).readlines()

idx = 0
for m, inchi_std in zip(molit, inchiit):
    idx += 1
    try:
        print("%d:\n" % (idx))
        print("  " + m.smiles())
        inchi_ind = indigo_inchi.getInchi(m)
        print("  InChI: " + inchi_ind)
        inchi_std = inchi_std.strip()
        if inchi_ind != inchi_std:
            print("  Error:\n    %s\n    %s\n" % (inchi_ind, inchi_std))
    except IndigoException as e:
        print("Error: %s\n" % (getIndigoExceptionText(e)))
        # print("InChI warning: %s\n" % (indigo_inchi.getWarning()))
        # print("InChI log: %s\n" % (indigo_inchi.getLog()))
