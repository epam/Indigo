
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

indigo.setOption("molfile-saving-skip-date", True)

print("*** Load ChemAxon CML format ***")
root = joinPath("molecules/chemaxon_cml")
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(joinPath(root, filename))
        print(mol.molfile())
        print(mol.cml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
