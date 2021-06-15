import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("molfile-saving-add-stereo-desc", "1")
print("This test checks correct determination and saving of CIP stereo descriptors")

for filename in sorted(os.listdir(joinPath("molecules/CIP"))):
    mol = indigo.loadMoleculeFromFile(os.path.join(joinPath("molecules/CIP"), filename))
    print("%s" % filename[:-4])
    print(mol.molfile())

indigo.setOption("ignore-stereochemistry-errors", "true")
rxn = indigo.loadReactionFromFile(joinPath("reactions/crazystereo.rxn"))
print(rxn.rxnfile())
