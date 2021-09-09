
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

indigo.setOption("molfile-saving-skip-date", True)

print("*** Load ChemAxon CML format ***")
root = joinPathPy("molecules/chemaxon_cml", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
        print(mol.cml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
