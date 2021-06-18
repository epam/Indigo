import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-noncritical-query-features", "true")

indigo_inchi = IndigoInchi(indigo)

files = [
    ("../../../../../data/molecules/basic/pubchem_slice_5000.smi", indigo.iterateSmilesFile),
    ("molecules/stereo.smi", indigo.iterateSmilesFile),
    ("molecules/set01.sdf", indigo.iterateSDFile)
]


def loadMol(data):
    indigo.setOption("ignore-stereochemistry-errors", "0")
    try:
        return indigo.loadMolecule(data)
    except IndigoException as e:
        print("  Exception: %s" % (getIndigoExceptionText(e)))
        indigo.setOption("ignore-stereochemistry-errors", "1")
        return indigo.loadMolecule(data)


for file, method in files:
    print("*** %s ***" % (file))
    idx = 0
    for m2 in method(joinPath(file)):
        idx += 1
        print("%d:" % (idx))

        m = loadMol(m2.rawData())
        try:
            print("  Smiles1: " + m.smiles())
            m.aromatize()
            csmiles = m.canonicalSmiles()
            if csmiles == "":
                continue
            print("  CSmiles1: " + csmiles)
            inchi = indigo_inchi.getInchi(m)
            print("  InChI1: " + inchi)
            m2 = indigo_inchi.loadMolecule(inchi)
            print("  Smiles2: " + m2.smiles())
            m2.aromatize()
            csmiles2 = m2.canonicalSmiles()
            print("  CSmiles2: " + csmiles2)
            inchi2 = indigo_inchi.getInchi(m2)
            print("  InChI2: " + inchi2)
            if inchi != inchi2:
                print("Warning: InChIs are different:\n  %s\n  %s\n" % (inchi, inchi2))
            if csmiles != csmiles2:
                print("Warning: CSmiles are different:\n  %s\n  %s\n" % (csmiles, csmiles2))
            print("  InChI key: " + indigo_inchi.getInchiKey(inchi))
            warn = indigo_inchi.getWarning()
            if len(warn) > 0:
                print("  Warning: " + warn)

        except IndigoException as e:
            print("Error: %s" % (getIndigoExceptionText(e)))
            print("InChI warning: %s" % (indigo_inchi.getWarning()))
            print("InChI log: %s" % (indigo_inchi.getLog()))
