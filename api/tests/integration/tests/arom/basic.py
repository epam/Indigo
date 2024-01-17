import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("****** Arom/Dearom ********")
m = indigo.loadMolecule("[As]1C=N[AsH]S=1C")
origin_smiles = m.smiles()
print(origin_smiles)
m.aromatize()
print(m.smiles())
m.dearomatize()
restored_smiles = m.smiles()
print(restored_smiles)

if origin_smiles != restored_smiles:
    sys.stderr.write("%s != %s" % (origin_smiles, restored_smiles))


def processMolecule(m):
    sm1 = m.canonicalSmiles()
    print(sm1)
    try:
        print("Arom:")
        m.aromatize()
    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))
    try:
        print("Dearom:")
        m.dearomatize()
    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))
    sm2 = m.canonicalSmiles()
    print(sm2)


print("***** Invalid valence *****")
processMolecule(indigo.loadMolecule("I1c2ccccc2c3ccccc13"))

print("***** SMILES with special aromatic atoms *****")
mols = [
    "[si]1(c(OC)c(c(C)cc1)c2ccccc2)OC",  # Cactvs: [si]
    "c1ccc2[as]c3ccccc3[siH]c2c1",
    "c1ccc2[te]c3ccccc3[BH]c2c1",  # RDKit: [te]
    "C[B]1o[B](C)o[B](C)o1",
]
for smiles in mols:
    print("***\n%s: " % (smiles))
    try:
        processMolecule(indigo.loadMolecule(smiles))
    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))

print("***** Other cases *****")


def processToDearomatize(m):
    try:
        print(" " + m.smiles())
        print(" " + m.canonicalSmiles())
        m.dearomatize()
        print(" " + m.smiles())
        print(" " + m.canonicalSmiles())
    except IndigoException as err:
        print("  %s" % (getIndigoExceptionText(err)))


for m in indigo.iterateSmilesFile(
    joinPathPy("molecules/dearomatization.smi", __file__)
):
    print(m.rawData())
    processToDearomatize(m)
    m2 = indigo.loadMolecule(m.molfile())
    processToDearomatize(m2)

print("***** Radical *****")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/benzene_radical.mol", __file__)
)
print(m.smiles())
print("Aromatize")
try:
    m.aromatize()
    print("  " + m.smiles())
except IndigoException as err:
    print("  %s" % (getIndigoExceptionText(err)))
print("Dearomatize")
try:
    m.dearomatize()
    print("  " + m.smiles())
except IndigoException as err:
    print("  %s" % (getIndigoExceptionText(err)))
print("Aromatize")
try:
    m.aromatize()
    print("  " + m.smiles())
except IndigoException as err:
    print("  %s" % (getIndigoExceptionText(err)))

print("***** Valences *****")
mol = indigo.loadMolecule("I1c2ccccc2c3ccccc13")
print(mol.smiles())
try:
    mol.aromatize()
    print(mol.smiles())
except IndigoException as err:
    print("  %s" % (getIndigoExceptionText(err)))
mol.dearomatize()
print(mol.smiles())
try:
    mol.aromatize()
    print(mol.smiles())
except IndigoException as err:
    print("  %s" % (getIndigoExceptionText(err)))

print("***** Coordination compound *****")


def executeOperation(m, func, msg):
    try:
        func(m)
        print(msg + m.smiles())

    except IndigoException as e:
        print(msg + getIndigoExceptionText(e))


def arom(m):
    m.aromatize()


def dearom(m):
    m.dearomatize()


def noneFunc(m):
    pass


indigo = Indigo()
for root, dirnames, filenames in os.walk(
    joinPathPy("molecules/arom-test", __file__)
):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s: \n" % filename)
        try:
            m1 = indigo.loadMoleculeFromFile(os.path.join(root, filename))
            m2 = indigo.loadMoleculeFromFile(os.path.join(root, filename))

            executeOperation(m1, noneFunc, "  Original:     ")
            executeOperation(m1, arom, "  Arom:         ")
            executeOperation(m2, dearom, "  Dearom:       ")
            executeOperation(m1, dearom, "  Arom->Dearom: ")
            executeOperation(m2, arom, "  Dearom->Arom: ")

        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

print("***** R-group fragments *****")


def printMoleculeWithRGroups(m):
    print(m.smiles())
    print("RGroup count: %d" % m.countRGroups())
    for rg in m.iterateRGroups():
        print("RGroup=%d:" % rg.index())
        for fr in rg.iterateRGroupFragments():
            print("  Fragment=%d:" % fr.index())
            print("    " + fr.smiles())


mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/arom_rgroup_member.mol", __file__)
)
printMoleculeWithRGroups(mol)
print("")
print("Aromatized:")
mol.aromatize()
printMoleculeWithRGroups(mol)

print("***** Number of hydrogens when loading from SMILES *****")
orginal = "Cc1nnc2c(N)ncnc12"
print(orginal)
m = indigo.loadMolecule("Cc1nnc2c(N)ncnc12")
print(m.smiles())
try:
    print(m.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
try:
    m.dearomatize()
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))

try:
    print(m.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
m.aromatize()
try:
    print(m.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
m2 = indigo.loadMolecule(m.smiles())
try:
    print(m2.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
try:
    m2.dearomatize()
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
try:
    print(m2.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))
m.aromatize()
try:
    print(m.canonicalSmiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))

print("***** Dearomatize -> Aromatize *****")
m = indigo.loadMolecule("OC(C1=C(N)N=CN1)=O")
print(m.smiles())
m.aromatize()
indigo.setOption("unique-dearomatization", "true")
try:
    m.dearomatize()
    print(m.smiles())
except IndigoException as e:
    print("  %s" % (getIndigoExceptionText(e)))


print("***** Bridge bond  *****")
m = indigo.loadMolecule("CC1=CC2=CNC=CC2=N1")
print(m.smiles())
m.aromatize()
print(m.smiles())
m.dearomatize()
print(m.smiles())

print("***** Invalid connectivity *****")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/invalid-connectivity.mol", __file__)
)
print(m.smiles())
m.aromatize()
print(m.smiles())
m.dearomatize()
print(m.smiles())

print("***** Select dearomatization with higher number of double bonds *****")
indigo = Indigo()
m = indigo.loadMolecule("c1cnn2nnnc2c1")
print(m.smiles())
m.dearomatize()
print(m.smiles())
m.aromatize()
print(m.smiles())

print("***** Arom and cis-trans *****")
indigo = Indigo()
for model in ["basic", "generic"]:
    print(model)
    indigo.setOption("aromaticity-model", model)
    m = indigo.loadMolecule("Cn1c2ccccc2c(-c2ccccc2)n/c(=N\\O)c1=O")
    print(m.smiles())
    m.dearomatize()
    print(m.smiles())
    m.aromatize()
    print(m.smiles())

print("***** Process arom atoms  *****")
indigo = Indigo()

for mol in indigo.iterateSDFile(
    joinPathPy("molecules/issue_22.sdf", __file__)
):
    mol.dearomatize()
    print(mol.smiles())

print("***** Process ferrocene-like structure  *****")
indigo = Indigo()
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/BoPhoz(R).mol", __file__)
)
m.aromatize()
print(m.smiles())

q = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/BoPhoz(R).mol", __file__)
)
q.aromatize()
print(q.smiles())

matcher = indigo.substructureMatcher(m)
assert matcher.match(q) != None


print("***** Dearomatization for R-Groups  *****")

indigo = Indigo()
mol = indigo.loadMoleculeFromFile(
    joinPathPy(
        "../../../../../data/molecules/rgroups/Rgroup_for_Dearomatize.mol",
        __file__,
    )
)

mol.aromatize()
print("before")
for rgroup in mol.iterateRGroups():
    print("  Rgroup #" + str(rgroup.index()))
    for frag in rgroup.iterateRGroupFragments():
        print(frag.canonicalSmiles())

mol.dearomatize()
print("after dearom")

for rgroup in mol.iterateRGroups():
    print("  Rgroup #" + str(rgroup.index()))
    for frag in rgroup.iterateRGroupFragments():
        print(frag.canonicalSmiles())

print("***** dearomatize-on-load option test  *****")
indigo.setOption("molfile-saving-skip-date", "1")
styr = indigo.loadMolecule("c1c(C=C)cccc1")
styr.aromatize()
styr.layout()
aromatized_styr = styr.molfile()
styr = indigo.loadMolecule(aromatized_styr)
# by default it should stay aromatized
print(styr.molfile())
indigo.setOption("dearomatize-on-load", "true")
styr = indigo.loadMolecule(aromatized_styr)
# should be dearomatized
print(styr.molfile())
indigo.setOption("dearomatize-on-load", "false")
styr = indigo.loadMolecule(aromatized_styr)
# should be aromatized
print(styr.molfile())
# should not dearomatize queries
indigo.setOption("dearomatize-on-load", "true")
q = indigo.loadQueryMolecule("[#6]=,:[#6]")
print(q.json())

print("***** Dearomatize molecule with custom query without label  *****")
m = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/issue_1524.ket", __file__)
)
m.dearomatize()
print(m.smarts())

print(
    "***** Dearomatize molecule with atom_aromatic_connectivity < 0 should not cause exception  *****"
)
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/issue_1478.ket", __file__)
)
m.dearomatize()
print(m.smiles())
