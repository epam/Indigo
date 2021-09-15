import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
saver = indigo.createFileSaver(joinPathPy("out/layout_hardmols.sdf", __file__), "SDF")

chebi_path = joinPathPy("../../../../../data/molecules/chebi", __file__)

def testLayout():
    indigo.setOption("layout-max-iterations", 1)
    indigo.setOption("skip-3d-chirality", True)
    files = os.listdir(chebi_path)
    files.sort()
    for filename in files:
        mol = indigo.loadMoleculeFromFile(os.path.join(chebi_path, filename))
        mol.setName(filename)
        print(relativePath(os.path.join(chebi_path, filename)))
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
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/layout-timeout.mol", __file__))

for it in [1, 2, 3, 32, 64, 0]:
    indigo.setOption("layout-max-iterations", it)
    print("layout-max-iterations: %d" % it)
    try:
        m.layout()
        saver.append(m)
    except IndigoException as e:
        print("Exception: " + getIndigoExceptionText(e))
