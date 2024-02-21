import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def iterateSavingOptions():
    for val in ["auto", "2000", "3000"]:
        print(" molfile-saving-mode: %s" % (val))
        indigo.setOption("molfile-saving-mode", val)
        yield val


def testReload(mol):
    molfile = mol.molfile()
    mol2 = indigo.loadMolecule(molfile)
    molfile2 = mol2.molfile()
    if molfile != molfile:
        print("Molecule is different after resave")
        sys.stderr.write("Molecule is different after resave")


indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)
if not os.access(joinPathPy("out", __file__), os.F_OK):
    os.mkdir(joinPathPy("out", __file__))
saver = indigo.createFileSaver(
    joinPathPy("out/mol_features.sdf", __file__), "sdf"
)
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/sgroups_2.mol", __file__)
)
for opt in iterateSavingOptions():
    print(mol.molfile())
    saver.append(mol)

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/all_features_mol.mol", __file__)
)
for opt in iterateSavingOptions():
    print(mol.molfile())
    saver.append(mol)

print("Iterate Groups")


def getGroups(m):
    for g in m.iterateDataSGroups():
        yield "data", g, m.getDataSGroup(g.index())
    for g in m.iterateGenericSGroups():
        yield "generic", g, m.getGenericSGroup(g.index())
    for g in m.iterateSuperatoms():
        yield "super", g, m.getSuperatom(g.index())
    for g in m.iterateMultipleGroups():
        yield "mutiple", g, m.getMultipleGroup(g.index())
    for g in m.iterateRepeatingUnits():
        yield "repeating", g, m.getRepeatingUnit(g.index())


def printGroupsInfo(m):
    for type, g, g2 in getGroups(m):
        print("%s %d" % (type, g.index()))
        for a, a2 in zip(g.iterateAtoms(), g2.iterateAtoms()):
            if a.index() != a2.index():
                sys.stderr.write("%d != %d" % (a.index(), a2.index()))
            print("  atom %d" % a.index())
        if type == "data":
            print("  description =  " + g.description())
            print("  data =  " + g.data())


printGroupsInfo(mol)

print("*** SGroup hierarchy ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/indsp-144-non-stoichio_test1.mol", __file__)
)
printGroupsInfo(mol)

print("*** Checking different MOLFILE features from the specification ***")
for mol in indigo.iterateSDFile(
    joinPathPy("molecules/check_specification.sdf", __file__)
):
    try:
        print(mol.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))

print("")
print("*** Load query molecules from Molfiles ***")
saver = indigo.createFileSaver(
    joinPathPy("out/query-molfile.sdf", __file__), "sdf"
)
for root, dirnames, filenames in os.walk(
    joinPathPy("molecules/query-molfile", __file__)
):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s:\n" % filename)
        try:
            mol = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename)
            )
            for opt in iterateSavingOptions():
                print(mol.molfile())
                saver.append(mol)
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

print("*** Test to load large properties ***")
for root, dirnames, filenames in os.walk(
    joinPathPy("molecules/ind-459", __file__)
):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s:\n" % filename)
        for mol in indigo.iterateSDFile(os.path.join(root, filename)):
            try:
                print(mol.smiles())
                for prop in mol.iterateProperties():
                    print("  ** %s: %s" % (prop.name(), prop.rawData()))
            except IndigoException as e:
                print("  %s" % (getIndigoExceptionText(e)))

print("*** Long and multiline SGroup data ***")
saver = indigo.createFileSaver(
    joinPathPy("out/multiline.sdf", __file__), "sdf"
)
mols = [
    "molecules/multiline-sgroups-ketcher-457-v2000.mol",
    "molecules/multiline-sgroups-ketcher-457-empty.mol",
    "molecules/multiline-sgroups-ketcher-457-v3000.mol",
    "molecules/multiline-sgroups-ketcher-457-single.mol",
]
for molfile in mols:
    print(molfile)
    mol = indigo.loadMoleculeFromFile(joinPathPy(molfile, __file__))
    for opt in iterateSavingOptions():
        print(mol.molfile())
        saver.append(mol)
        testReload(mol)


print("*** Molfile properties ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/mol-with-prop.mol", __file__)
)
print(mol.smiles())
for prop in mol.iterateProperties():
    print(prop.name() + ":" + prop.rawData())

print("*** Abbreviation attachement points ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/new-v3000-sap.mol", __file__)
)
print(mol.smiles())
print(mol.molfile())

print("*** 2-digit pseudoatom index ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/invalid_pseudo.mol", __file__)
)
print(mol.smiles())

print("*** 1415 wrong S-Group count in molfile V3000 ***")
smiles = "C1CCC(C1)Nc1ccc2ccn(-c3ccc4[nH]ncc4c3)c2c1"
mol = indigo.loadMolecule(smiles)
print(smiles)
indigo.setOption("molfile-saving-mode", "3000")
print(mol.molfile())

print("*** 1431 Query explicit valency crash ***")
qmol = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/query_crash_1431.mol", __file__)
)

print(qmol.smarts())

print("\n*** 1468 MRV SMA overrite valence ***")
filename = joinPathPy("molecules/issue_1468.mol", __file__)
qmol = indigo.loadQueryMoleculeFromFile(filename)
with open(filename) as f:
    mol_origin = f.read()
indigo.setOption("molfile-saving-mode", "2000")
mol_save = qmol.molfile()
if mol_save == mol_origin:
    print("Succes.\n")
else:
    print("Failure. \n%s\ndifferent from\n%s\n" % (mol_origin, mol_save))
