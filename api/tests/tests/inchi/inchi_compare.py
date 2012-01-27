import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo_inchi = IndigoInchi(indigo)

idx = 0
for m in indigo.iterateSmilesFile("../../data/pubchem_slice_5000.smi"):
    idx += 1
    try:
        print("%d:" % (idx))
        csmiles = m.canonicalSmiles()
        if csmiles == "":
            continue
        print("  CSmiles1: " + csmiles)
        inchi = indigo_inchi.getInchi(m)
        print("  InChI1: " + inchi)
        m2 = indigo_inchi.loadMolecule(inchi)
        csmiles2 = m.canonicalSmiles()
        print("  CSmiles2: " + csmiles2)
        inchi2 = indigo_inchi.getInchi(m2)
        print("  InChI2: " + inchi2)
        if inchi != inchi2:
            print("Error: InChIs are different:\n  %s\n  %s\n" % (inchi, inchi2))
        if csmiles != csmiles2:
            print("Error: CSmiles are different:\n  %s\n  %s\n" % (csmiles, csmiles2))
        print("  InChI key: " + indigo_inchi.getInchiKey(inchi))
    except IndigoException, e:
        print("Error: %s\n" % (getIndigoExceptionText(e)))
        print("InChI warning: %s\n" % (indigo_inchi.getWarning()))
        print("InChI log: %s\n" % (indigo_inchi.getLog()))
    