import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

print("****** Check wrong chiral flag ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/wrong_chiral_flag.mol"))
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())


print("****** Check stereogenic center ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/stereogenic.mol"))
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())


print("****** Check 3D stereo center ********")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/stereo_3d.mol"))
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())

