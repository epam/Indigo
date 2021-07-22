import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
saver = indigo.createFileSaver(joinPath("out/layout_hardmols.sdf"), "SDF")


def testLayout():
    indigo.setOption("layout-max-iterations", 1)
    indigo.setOption("skip-3d-chirality", True)
    files = os.listdir(joinPath("../../../../../data/molecules/chebi"))
    files.sort()
    for filename in files:
        mol = indigo.loadMoleculeFromFile(joinPath("../../../../../data/molecules/chebi", filename))
        mol.setName(filename)
        print(relativePath(joinPath("../../../../../data/molecules/chebi", filename)))
        try:
            mol.layout()
        except IndigoException as e:
            print("Exception: " + getIndigoExceptionText(e) + " on " + mol.name())
        saver.append(mol)

        print(mol.smiles())
        newmol = indigo.loadMolecule(mol.smiles())
        newmol.setName(filename + " (from smiles)")
        try:
            newmol.layout()
        except IndigoException as e:
            print("Exception: " + getIndigoExceptionText(e) + " on " + newmol.name())
        saver.append(newmol)


testLayout()

print("*** Layout with timeout ***")
indigo.setOption("timeout", "100")
m = indigo.loadMoleculeFromFile(joinPath("molecules", "layout-timeout.mol"))

for it in [1, 2, 3, 32, 64, 0]:
    indigo.setOption("layout-max-iterations", it)
    print("layout-max-iterations: %d" % it)
    try:
        m.layout()
        saver.append(m)
    except IndigoException as e:
        print("Exception: " + getIndigoExceptionText(e))
