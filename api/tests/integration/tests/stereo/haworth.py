import sys
import glob
from os.path import basename

sys.path.append('../../common')
from env_indigo import *

def listFiles (pattern):
    files = list(glob.glob(pattern))
    return sorted(files)

indigo = Indigo()

print("****** Compare with references ********")

def setStereo(m):
    for s in m.iterateStereocenters():
        if s.stereocenterType() == Indigo.AND:
            s.changeStereocenterType(Indigo.ABS)
        if s.stereocenterType() == Indigo.EITHER:
            s.resetStereo()

indigo.setOption("stereochemistry-detect-haworth-projection", "true")

for name in listFiles(joinPathPy("molecules/haworth", __file__) + "/*.mol"):
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

print("****** Check search ********")
indigo = Indigo()

def checkSub (qname, tname):
    q = indigo.loadQueryMoleculeFromFile(joinPathPy(qname, __file__))
    t = indigo.loadMoleculeFromFile(joinPathPy(tname, __file__))
    matcher = indigo.substructureMatcher(t)
    return matcher.match(q) != None

indigo.setOption("stereochemistry-detect-haworth-projection", "true")
assert checkSub("molecules/projection_ordinary.mol", "molecules/projection_haworth.mol")
assert checkSub("molecules/projection_haworth.mol", "molecules/projection_ordinary.mol")
