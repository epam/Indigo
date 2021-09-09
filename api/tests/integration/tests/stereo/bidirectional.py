import sys
import glob
from os.path import basename

sys.path.append('../../common')
from env_indigo import *

def listFiles (pattern):
    files = list(glob.glob(pattern))
    return sorted(files)

indigo = Indigo()
bidirectional_dir = joinPathPy("molecules/bidirectional", __file__)
print("****** Load without ignore errors ********")
for name in listFiles(bidirectional_dir + "/*.mol"):
    sys.stdout.write(basename(name) + " ")
    try:
        m = indigo.loadMoleculeFromFile(name)
        print(m.smiles())
    except IndigoException as e:
        print("%s" % (getIndigoExceptionText(e)))

print("****** Load with ignore errors ********")
indigo.setOption("ignore-stereochemistry-errors", True)
for name in listFiles(bidirectional_dir + "/*.mol"):
    sys.stdout.write(basename(name) + " ")
    m = indigo.loadMoleculeFromFile(name)
    print(m.smiles())

print("****** Load with bidirectional mode ********")
indigo.setOption("ignore-stereochemistry-errors", False)
indigo.setOption("stereochemistry-bidirectional-mode", True)
for name in listFiles(bidirectional_dir + "/*.mol"):
    sys.stdout.write(basename(name) + " ")
    try:
        m = indigo.loadMoleculeFromFile(name)
        print(m.smiles())
    except IndigoException as e:
        print("%s" % (getIndigoExceptionText(e)))

print("****** Compare with reference in bidirectional mode ********")

def setStereo(m):
    for s in m.iterateStereocenters():
        if s.stereocenterType() == Indigo.AND:
            s.changeStereocenterType(Indigo.ABS)
        if s.stereocenterType() == Indigo.EITHER:
            s.resetStereo()

indigo.setOption("stereochemistry-bidirectional-mode", True)
indigo.setOption("ignore-stereochemistry-errors", False)
for name in listFiles(bidirectional_dir + "/*.mol"):
    sys.stdout.write(basename(name) + " ")
    smi_file = name.rpartition('.')[0] + ".smi"
    try:
        m = indigo.loadMoleculeFromFile(name)
        setStereo(m)
        print(m.smiles())

        sm = indigo.loadMoleculeFromFile(smi_file)
        cs1 = m.canonicalSmiles()
        cs2 = sm.canonicalSmiles()
        if cs1 != cs2:
            print("  Different:")
            print("    " + cs1)
            print("    " + cs2)

    except IndigoException as e:
        print("%s" % (getIndigoExceptionText(e)))
