import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("****** Check wrong chiral flag ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/wrong_chiral_flag.mol", __file__)
)
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())


print("****** Check stereogenic center ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereogenic.mol", __file__)
)
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())


print("****** Check 3D stereo center ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereo_3d.mol", __file__)
)
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centetrs exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())
