import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

print("****** Get valence ********")
def getMoleculesValence (iter):
    for num, mol in enumerate(iter):
        print("molecule #%d: " % (num))
        for atom in mol.iterateAtoms():
            msg = atom.checkBadValence()
            if len(msg) > 0:
               print("Atom index %d has bad valence: %s" % (atom.index(), msg))
        msg = mol.checkBadValence()
        if len(msg) > 0:
            print(msg)
        msg = mol.checkAmbiguousH()
        if len(msg) > 0:
            print(msg)


getMoleculesValence(indigo.iterateSDFile(joinPath("molecules/valence_test1.sdf")))


print("****** Set explicit valence ********")
m = indigo.loadMolecule("CP(C)C")
print(m.smiles())
a = m.getAtom(1)
print(a.valence())
a.setExplicitValence(6)
print(a.valence())
print(m.smiles())
print(m.molfile())

print("****** Explicit unusual valence ********")
getMoleculesValence(indigo.iterateSDFile(joinPath("../../../../../data/molecules/basic/explicit_valence.sdf")))
