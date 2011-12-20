import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
if not os.path.exists(joinPath("out")):
    os.makedirs(joinPath("out"))
saver = indigo.createFileSaver(joinPath("out/mol_features.sdf"), "sdf")
mol = indigo.loadMoleculeFromFile(joinPath('molecules/sgroups_2.mol'))
print(mol.molfile())
saver.append(mol)
mol = indigo.loadMoleculeFromFile(joinPath('molecules/all_features_mol.mol'))
print(mol.molfile())
saver.append(mol)

print("Checking different MOLFILE features from the specification")
for mol in indigo.iterateSDFile(joinPath('molecules/check_specification.sdf')):
    try:
        print(mol.canonicalSmiles())
    except IndigoException, e:
        print(getIndigoExceptionText(e))
    