
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

print("*** CDXML to mol ***")

root = joinPathPy("molecules/cdxml2", __file__)
for filename in sorted(os.listdir(root)):
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
