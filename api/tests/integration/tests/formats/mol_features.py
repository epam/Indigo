import sys, os
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

def iterateSavingOptions ():
    for val in [ "auto", "2000", "3000" ]:
        print(" molfile-saving-mode: %s" % (val))
        indigo.setOption("molfile-saving-mode", val)
        yield val

def testReload (mol):
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
saver = indigo.createFileSaver(joinPathPy("out/mol_features.sdf", __file__), "sdf")
mol = indigo.loadMoleculeFromFile(joinPathPy('molecules/sgroups_2.mol', __file__))
for opt in iterateSavingOptions():
    print(mol.molfile())
    saver.append(mol)
    
mol = indigo.loadMoleculeFromFile(joinPathPy('molecules/all_features_mol.mol', __file__))
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

def printGroupsInfo (m):
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
mol = indigo.loadMoleculeFromFile(joinPathPy('molecules/indsp-144-non-stoichio_test1.mol', __file__))
printGroupsInfo(mol)
    
print("*** Checking different MOLFILE features from the specification ***")
for mol in indigo.iterateSDFile(joinPathPy('molecules/check_specification.sdf', __file__)):
    try:
        print(mol.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    
print("")
print("*** Load query molecules from Molfiles ***")
saver = indigo.createFileSaver(joinPathPy("out/query-molfile.sdf", __file__), "sdf")
for root, dirnames, filenames in os.walk(joinPathPy("molecules/query-molfile", __file__)):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s:\n" % filename)
        try:
            mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
            for opt in iterateSavingOptions():
                print(mol.molfile())
                saver.append(mol)
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

print("*** Test to load large properties ***")
for root, dirnames, filenames in os.walk(joinPathPy("molecules/ind-459", __file__)):
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
saver = indigo.createFileSaver(joinPathPy("out/multiline.sdf", __file__), "sdf")
mols = [ 
    'molecules/multiline-sgroups-ketcher-457-v2000.mol',
    'molecules/multiline-sgroups-ketcher-457-empty.mol',
    'molecules/multiline-sgroups-ketcher-457-v3000.mol',
    'molecules/multiline-sgroups-ketcher-457-single.mol',
]
for molfile in mols:
    print(molfile)
    mol = indigo.loadMoleculeFromFile(joinPathPy(molfile, __file__))
    for opt in iterateSavingOptions():
        print(mol.molfile())
        saver.append(mol)
        testReload(mol)

        
print("*** Molfile properties ***")
mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/mol-with-prop.mol", __file__))
print(mol.smiles())
for prop in mol.iterateProperties():
    print(prop.name() + ":" + prop.rawData())
    
print("*** Abbreviation attachement points ***")
mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/new-v3000-sap.mol", __file__))
print(mol.smiles())
print(mol.molfile())

print("*** 2-digit pseudoatom index ***")
mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/invalid_pseudo.mol", __file__))
print(mol.smiles())

print('*** INDIGO_ALIAS molfile ***')
mol = indigo.loadMolecule('''
  Ketcher 08151618402D 1   1.00000     0.00000     0

 13 13  0     0  0            999 V2000
   -0.8662    1.5003    0.0000 C   0  0  0  0  0  7  0  0  0  0  0  0
   -1.7324    1.0003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7324    0.0001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.8662   -0.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0001    1.0002    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5982    1.5002    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.8659    1.5001    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.8663   -1.4999    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    0.5876   -0.8089    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.9943   -0.1045    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.2079   -0.9779    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.7431    0.6690    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  1  6  2  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  4  5  2  0     0  0
  5  6  1  0     0  0
  2  7  1  0     0  0
  6  8  1  0     0  0
  4  9  1  0     0  0
  5 10  1  0     0  0
  5 11  1  0     0  0
  5 12  1  0     0  0
  5 13  1  0     0  0
M  CHG  1   3 -40
M  RAD  1   3   3
M  STY  1   1 DAT
M  SLB  1   1   1
M  SAL   1  1   7
M  SDT   1 INDIGO_ALIAS
M  SDD   1    -2.5982    1.5002    AA    ALL  1      1
M  SED   1 Psd
M  STY  1   2 DAT
M  SLB  1   2   2
M  SAL   2  1   8
M  SDT   2 INDIGO_ALIAS
M  SDD   2     0.8659    1.5001    AA    ALL  1      1
M  SED   2 Pol
M  END
''')

indigo.setOption("molfile-saving-mode", "2000")
print(mol.molfile())

indigo.setOption("molfile-saving-mode", "3000")
print(mol.molfile())

print(mol.cml())

print("*** Single atom NOT list ***")
mol = indigo.loadQueryMoleculeFromFile(joinPathPy("molecules/single_not_list.mol", __file__))
print(mol.molfile())
