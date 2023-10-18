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

print("****** Check bad valence ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/issue_243.mol", __file__)
)

print("Check result (molecule) = %s" % mol.check("VALENCE"))

print("Check result (molecule) = %s" % mol.check())

print("Check result (molecule) = %s" % mol.check("ALL,-VALENCE"))

print("Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1 2"))

print("Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1 4"))

print("Check result (molecule) = %s" % mol.check("VALENCE, BONDS:1 2"))

print(
    "Check result (molecule) = %s" % mol.check("VALENCE, ATOMS:1, BONDS:1 2")
)

for atom in mol.iterateAtoms():
    print(
        "Check result (atom) %d = %s" % (atom.index(), atom.check("valence"))
    )
for bond in mol.iterateBonds():
    print(
        "Check result (bond) %d = %s" % (bond.index(), bond.check("VALENCE"))
    )


for idx, m in enumerate(
    indigo.iterateSDFile(joinPathPy("molecules/bad_valence.sdf", __file__))
):
    print("** %d **" % idx)
    print("Check result (molecule) = %s" % m.check())


print("****** Check chiral flag ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/wrong_chiral_flag.mol", __file__)
)
print("Check result (molecule) = %s" % mol.check())


print("****** Check stereogenic center ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereogenic.mol", __file__)
)
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centers exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())
print("Check result = %s" % mol.check())


print("****** Check 3D stereo center ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereo_3d.mol", __file__)
)
print("Structure is chiral      = %d" % mol.isChiral())
print("Chiral flag consistency  = %d" % mol.checkChirality())
print("3D stereo centers exist = %d" % mol.check3DStereo())
print("Undefined stereo centers exist = %d" % mol.checkStereo())
print("Check result = %s" % mol.check())

print("****** Check overlapped atoms/bonds ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/overlapped.mol", __file__)
)
print("Check result = %s" % mol.check())

print("****** Check query/stereo features ********")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/query_test.mol", __file__)
)
print("Check result = %s" % mol.check())


print("****** Check reactions ********")
indigo.setOption("ignore-stereochemistry-errors", "true")

for idx, m in enumerate(
    indigo.iterateRDFile(joinPathPy("reactions/rxns.rdf", __file__))
):
    print("** %d **" % idx)

    try:
        print("Check result (reaction) = %s" % m.check())
    except IndigoException as e:
        print(getIndigoExceptionText(e))


print("****** Check structure ********")
c = "C1=C(*)C=CC=C1"
print("Check result = %s" % indigo.checkStructure(c))

c = "bad structure"
print("Check result = %s" % indigo.checkStructure(c))

c = "sil"
print("Check result = %s" % indigo.checkStructure(c))

c = "benzene"
print("Check result = %s" % indigo.checkStructure(c))

print("****** Check query features ket with custom query bond ********")
filename = "molecules/ket_with_custom_query_bond.ket"
try:
    mol = indigo.loadMoleculeFromFile(joinPathPy(filename, __file__))
except Exception as e:
    if "queries" in str(e):
        mol = indigo.loadQueryMoleculeFromFile(joinPathPy(filename, __file__))
    else:
        print("Check failed: loaded as molecule.")
print("Check result = %s" % mol.check())
