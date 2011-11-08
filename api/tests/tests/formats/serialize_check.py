import sys
import binascii

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

if not os.path.exists("out"):
   os.makedirs("out")
saver = indigo.createFileSaver("out/serialize_check.sdf", "sdf")

all_features_mol = indigo.loadMoleculeFromFile("molecules/all_features_mol.mol")

# Add highlighting
for index in [1, 4, 5, 6, 7, 10, 40, 12, 13, 18, 20]:
    a = all_features_mol.getAtom(index)
    a.highlight()

for index in [5, 8, 1, 4, 5, 85, 10, 15, 112, 13, 2]:
    b = all_features_mol.getBond(index)
    b.highlight()

    
def processMol(mol):
    print(mol.smiles())
    cano_sm = mol.canonicalSmiles()
    print(cano_sm)
    print(mol.molfile())

    saver.append(mol)
    mol.layout()
    saver.append(mol)

    m2 = indigo.loadMolecule(cano_sm)
    print(m2.smiles())
    cano_sm2 = m2.canonicalSmiles()
    print(cano_sm2)
    print(m2.molfile())

    m2.layout()
    saver.append(m2)

    if cano_sm != cano_sm2:
        sys.stderr("Canonical smiles are different:\n%s\n%s\n" % (cano_sm, cano_sm2))

    # Output serialized data to check consistency
    hex_ser1 = binascii.hexlify(mol.serialize())
    print("Molecule:\n%s" % (hex_ser1))
    hex_ser2 = binascii.hexlify(m2.serialize())
    print("Reloaded molecule:\n%s" % (hex_ser2))

processMol(all_features_mol)    
all_features_mol.aromatize()
processMol(all_features_mol)    
