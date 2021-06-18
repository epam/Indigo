import os
import sys
sys.path.append('../../common')
from env_indigo import *

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

def processMolecule (m):
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
    "c1ccc2[te]c3ccccc3[bH]c2c1", # RDKit: [te]
    "C[b]1o[b](C)o[b](C)o1",
]
for smiles in mols:
    print("***\n%s: " % (smiles))
    try:
        processMolecule(indigo.loadMolecule(smiles))
    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))
        
print("***** Other cases *****")
def processToDearomatize (m):
    try:
        print(" " + m.smiles())
        print(" " + m.canonicalSmiles())
        m.dearomatize()
        print(" " + m.smiles())
        print(" " + m.canonicalSmiles())
    except IndigoException as err:
        print("  %s" % (getIndigoExceptionText(err)))
        
for m in indigo.iterateSmilesFile(joinPath("molecules", "dearomatization.smi")):
    print(m.rawData())
    processToDearomatize(m)
    m2 = indigo.loadMolecule(m.molfile())
    processToDearomatize(m2)

print("***** Radical *****")
m = indigo.loadMoleculeFromFile(joinPath("molecules", "benzene_radical.mol"))
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

def executeOperation (m, func, msg):
    try:
        func(m)
        print(msg + m.smiles())
    
    except IndigoException as e:
        print(msg + getIndigoExceptionText(e))

def arom (m):
    m.aromatize()
def dearom (m):
    m.dearomatize()
def noneFunc (m):
    pass
        
indigo = Indigo()
for root, dirnames, filenames in os.walk(joinPath("molecules/arom-test")):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s: \n" % filename)
        try:
            m1 = indigo.loadMoleculeFromFile(os.path.join(root, filename))
            m2 = indigo.loadMoleculeFromFile(os.path.join(root, filename))
            
            executeOperation(m1, noneFunc, "  Original:     ")
            executeOperation(m1, arom,     "  Arom:         ")
            executeOperation(m2, dearom,   "  Dearom:       ")
            executeOperation(m1, dearom,   "  Arom->Dearom: ")
            executeOperation(m2, arom,     "  Dearom->Arom: ")
            
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

print("***** R-group fragments *****")
def printMoleculeWithRGroups (m):
    print(m.smiles())
    print('RGroup count: %d' % m.countRGroups())
    for rg in m.iterateRGroups():
        print("RGroup=%d:" % rg.index())
        for fr in rg.iterateRGroupFragments():
            print("  Fragment=%d:" % fr.index())
            print("    " + fr.smiles())

mol = indigo.loadMoleculeFromFile(joinPath("molecules/arom_rgroup_member.mol")) 
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
m = indigo.loadMoleculeFromFile(joinPath("molecules", "invalid-connectivity.mol"))
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
    m = indigo.loadMolecule("Cn1c2ccccc2c(-c2ccccc2)n/c(=N\O)c1=O")
    print(m.smiles())
    m.dearomatize()
    print(m.smiles())
    m.aromatize()
    print(m.smiles())

print("***** Process arom atoms  *****")
indigo = Indigo()

for mol in indigo.iterateSDFile(joinPath("molecules", "issue_22.sdf")):
    mol.dearomatize()
    print(mol.smiles())

print("***** Process ferrocene-like structure  *****")
indigo = Indigo()
m = indigo.loadMoleculeFromFile(joinPath("molecules", "BoPhoz(R).mol"))
m.aromatize()
print(m.smiles())

q = indigo.loadQueryMoleculeFromFile(joinPath("molecules", "BoPhoz(R).mol"))
q.aromatize()
print(q.smiles())

matcher = indigo.substructureMatcher(m)
assert(matcher.match(q) != None)


print("***** Dearomatization for R-Groups  *****")

indigo = Indigo()
mol = indigo.loadMoleculeFromFile(joinPath("..", "..", "..", "..", "..", "data", "molecules", "rgroups", "Rgroup_for_Dearomatize.mol"))

mol.aromatize()
print("before")
for rgroup in mol.iterateRGroups():
    print('  Rgroup #' + str(rgroup.index()))
    for frag in rgroup.iterateRGroupFragments():
        print(frag.canonicalSmiles())

mol.dearomatize()
print("after dearom")

for rgroup in mol.iterateRGroups():
    print('  Rgroup #' + str(rgroup.index()))
    for frag in rgroup.iterateRGroupFragments():
        print(frag.canonicalSmiles())
