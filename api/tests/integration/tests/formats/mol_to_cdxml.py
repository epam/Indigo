
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("*** Mol to CDXML ***")

root = joinPath("molecules/cdxml")
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(joinPath(root, filename))
        print(mol.cdxml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(joinPath(root, filename))
        print(mol.cdxml())
