import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("****** Check bad valence ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/issue_243.mol"))

print("Check result (molecule) = %s" % mol.check("VALENCE"))

print("Check result (molecule) = %s" % mol.check())

print("Check result (molecule) = %s" % mol.check("ALL,-VALENCE"))

print("Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1 2"))

print("Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1 4"))

print("Check result (molecule) = %s" % mol.check("VALENCE, BONDS:1 2"))

print("Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1, BONDS:1 2"))

for atom in mol.iterateAtoms():
    print("Check result (atom) %d = %s" % (atom.index(), atom.check("valence")))
for bond in mol.iterateBonds():
    print("Check result (bond) %d = %s" % (bond.index(), bond.check("VALENCE")))


for idx, m in enumerate(indigo.iterateSDFile(joinPath("molecules", "bad_valence.sdf"))):
    print("** %d **" % idx)
    print("Check result (molecule) = %s" % m.check())


print("****** Check chiral flag ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/wrong_chiral_flag.mol"))
print("Check result (molecule) = %s" % mol.check())


print("****** Check stereogenic center ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/stereogenic.mol"))
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centers exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())
print("Check result = %s" % mol.check())


print("****** Check 3D stereo center ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/stereo_3d.mol"))
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centers exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())
print("Check result = %s" % mol.check())

print("****** Check overlapped atoms/bonds ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/overlapped.mol"))
print("Check result = %s" % mol.check())

print("****** Check query/stereo fetures ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/query_test.mol"))
print("Check result = %s" % mol.check())


print("****** Check reactions ********")
indigo.setOption("ignore-stereochemistry-errors", "true")

for idx, m in enumerate(indigo.iterateRDFile(joinPath("reactions", "rxns.rdf"))):
    print("** %d **" % idx)

    try:
       print("Check result (reaction) = %s" % m.check())
    except IndigoException as e:
       print(getIndigoExceptionText(e))


print("****** Check strcuture ********")
c = "C1=C(*)C=CC=C1"
print("Check result = %s" % indigo.checkStructure(c))

c = "bad structure"    
print("Check result = %s" % indigo.checkStructure(c))

c = "sil"    
print("Check result = %s" % indigo.checkStructure(c))

c = "benzene"    
print("Check result = %s" % indigo.checkStructure(c))
