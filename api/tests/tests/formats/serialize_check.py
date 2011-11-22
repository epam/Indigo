import sys
import binascii

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
saver = indigo.createFileSaver(joinPath("out/serialize_check.sdf"), "sdf")

all_features_mol = indigo.loadMoleculeFromFile(joinPath("molecules/all_features_mol.mol"))

# Add highlighting
for index in [1, 4, 5, 6, 7, 10, 40, 12, 13, 18, 20]:
    a = all_features_mol.getAtom(index)
    a.highlight()

for index in [5, 8, 1, 4, 5, 85, 10, 15, 112, 13, 2]:
    b = all_features_mol.getBond(index)
    b.highlight()

hex_output = open(joinPath("out/serialize_hex.txt"), "w")
    
def getMolProperties (mol):
    return { 
        "atoms" : mol.countAtoms(),
        "bonds" : mol.countBonds(),
        "attachment points" : mol.countAttachmentPoints(),
        "hydrogens" : mol.countHydrogens(),
        "superatoms" : mol.countSuperatoms(),
        "data Sgroups" : mol.countDataSGroups(),
        "repeating units" : mol.countRepeatingUnits(),
        "multiple groups" : mol.countMultipleGroups(),
        "generic Sgroups" : mol.countGenericSGroups(),
        "pseudoatoms" : mol.countPseudoatoms(),
        "rsites" : mol.countRSites(),
        "stereocenters" : mol.countStereocenters(),
        "allenecenters" : mol.countAlleneCenters(),
        "components" : mol.countComponents(),
        "sssr" : mol.countSSSR(),
        "heavy atoms" : mol.countHeavyAtoms(),
    }
    
def checkEqualProperties (p1, p2):
    print("* checkEqualProperties *")
    for k in p1.keys():
        message = ""
        if p1[k] != p2[k]:
            message = " <- Error"
            #sys.stderr.write("Molecule properties are different\n")
        print("{0}: {1} {2} {3}".format(k, p1[k], p2[k], message))

def getCanonicalSmilesOrEmpty(m):
    try:
        return m.canonicalSmiles()
    except IndigoException, e:
        return ""

def printSGroupsInfo (m):
    for gr in m.iterateSuperatoms():
        str = ""
        for a in gr.iterateAtoms():
            str += " " + a.symbol()
        print(str)
        
def processMol(mol):
    sm = ""
    try:
        sm = mol.smiles()
        print(sm)
    except IndigoException, e:
        print(getIndigoExceptionText(e))
        
    cano_sm = getCanonicalSmilesOrEmpty(mol)
    print(cano_sm)
    #print(mol.molfile())

    saver.append(mol)
    mol.layout()
    saver.append(mol)

    m2 = indigo.loadMolecule(sm)
    sm2 = ""
    try:
        sm2 = m2.smiles()
        print(sm2)
    except IndigoException, e:
        print(getIndigoExceptionText(e))
    cano_sm2 = getCanonicalSmilesOrEmpty(m2)
    print(cano_sm2)
    #print(m2.molfile())

    m2.layout()
    saver.append(m2)

    if cano_sm != cano_sm2 and cano_sm != "":
        sys.stderr.write("Canonical smiles are different:\n%s\n%s\n" % (cano_sm, cano_sm2))
        open(joinPath("out/cano_sm.smi", "w")).write(cano_sm)
        open(joinPath("out/cano_sm2.smi", "w")).write(cano_sm2)

    # Output serialized data to check consistency
    buf = mol.serialize()
    hex_ser1 = binascii.hexlify(buf)
    print("Molecule:\n%s" % (hex_ser1))
    buf2 = m2.serialize()
    hex_ser2 = binascii.hexlify(buf2)
    print("Reloaded molecule:\n%s" % (hex_ser2))
    
    hex_output.write(hex_ser1 + "\n")
    hex_output.write(hex_ser2 + "\n")
    
    # Reload molecule from serialized buffer
    mol_rel = indigo.unserialize(buf)
    m2_rel = indigo.unserialize(buf2)
    
    cano_sm_rel = getCanonicalSmilesOrEmpty(mol_rel)
    cano_sm2_rel = getCanonicalSmilesOrEmpty(m2_rel)
    
    if cano_sm != cano_sm_rel and cano_sm != "":
        sys.stderr.write("Canonical smiles are different after unserialize(serialize()):\n%s\n%s\n" % (cano_sm, cano_sm_rel))
        open(joinPath("out/cano_sm.smi", "w")).write(cano_sm)
        open(joinPath("out/cano_sm_rel.smi", "w")).write(cano_sm_rel)
    
    if cano_sm2 != cano_sm2_rel:
        sys.stderr.write("Canonical smiles are different after unserialize(serialize()):\n%s\n%s\n" % (cano_sm2, cano_sm2_rel))
        open(joinPath("out/cano_sm2.smi", "w")).write(cano_sm2)
        open(joinPath("out/cano_sm2_rel.smi", "w")).write(cano_sm2_rel)

    p1 = getMolProperties(mol)
    p1_rel = getMolProperties(mol_rel)
    
    checkEqualProperties(p1, p1_rel)
        
    p2 = getMolProperties(m2)
    p2_rel = getMolProperties(m2_rel)
    
    checkEqualProperties(p2, p2_rel)
    
    if mol.countSuperatoms() > 0:
        print("Superatoms:")
        printSGroupsInfo(mol)
        printSGroupsInfo(mol_rel)


processMol(all_features_mol)    
all_features_mol.aromatize()
processMol(all_features_mol)    

# Process other molecules
test_sets = [ 
    (joinPath("../../data/thiazolidines.sdf"), indigo.iterateSDFile),
    (joinPath("../../data/all-allenes.sdf"), indigo.iterateSDFile),
    (joinPath("../../data/all_sgroups.sdf"), indigo.iterateSDFile),
    ]
    
for file, func in test_sets:
    print("Molecules set: %s" % (file))
    it = func(file)
    for m, idx in zip(it, range(100000)):
        print("\nTesting molecule #%d" % (idx))
        try:
            processMol(m)    
            m.aromatize()
            processMol(m)    
        except IndigoException, e:
            print("caught {0}\n".format(getIndigoExceptionText(e)))
      