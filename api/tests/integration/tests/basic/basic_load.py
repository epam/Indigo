import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
mol = indigo.loadMolecule("OCC1C(O)C(O)C(O)C(OC2C(O)C(O)C(OCC3CCCCC3)OC2CO)O1 |ha:0,1,2,3,4,5,6,7,8,9,10,29,hb:0,1,2,3,4,5,6,7,8,9,30,31|")
print("****** Molfile 2000 ********")
indigo.setOption("molfile-saving-mode", "2000")
print(mol.molfile())
print("****** Molfile 3000 ********")
indigo.setOption("molfile-saving-mode", "3000")
print(mol.molfile())
print("****** CML ********")
print(mol.cml())
print("****** SMILES ********")
print(mol.smiles())
print("****** Canonical SMILES ********")
mol.unhighlight()
print(mol.canonicalSmiles())
print("****** Loading SDF with multiline properties ********")
for item in indigo.iterateSDFile(joinPathPy("molecules/multiline_properties.sdf", __file__)):
    for prop in item.iterateProperties():
        print(prop.name() + " : " + prop.rawData())

print("****** CurlySMILES ********")
m = indigo.loadMolecule("PC{-}{+n}N")      
print(m.smiles())
m = indigo.loadMolecule("PC{-}O{+n}N")      
print(m.smiles())

print("****** Finding invalid stereocenters ********")
for item in indigo.iterateSDFile(joinPathPy("molecules/invalid_3d_stereocenters.sdf", __file__)):
    try:
        print(item.molfile())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    try:
        print(item.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))

print("****** Extended aromatic SMILES ********")
m = indigo.loadMolecule("NC(Cc1c[nH]c2cc[te]c12)C(O)=O")
print(m.smiles())
m.dearomatize()
print(m.smiles())

print("****** Skip BOM flag ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/mol-utf8-bom.mol", __file__))
print(m.name())

print("****** Incomplete stereo in SMILES/SMARTS ********")
print("[*@]")
m = indigo.loadQueryMolecule("[*@]")
print(m.smiles())
print("[*@H]")
m = indigo.loadQueryMolecule("[*@H]")
print(m.smiles())
print("[*@H](~*)~*")
m = indigo.loadQueryMolecule("[*@H](~*)~*")
print(m.smiles())

print("****** H2 molecule ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/H2.mol", __file__))
indigo.setOption("molfile-saving-mode", "2000")
print(m.smiles())
print(m.canonicalSmiles())
print(m.molfile())
print(m.grossFormula())

print("****** S-group's SCL (CLASS) support ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/sa-class-v2000.mol", __file__))
indigo.setOption("molfile-saving-mode", "2000")
print(m.canonicalSmiles())
print(m.molfile())

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/sa-class-v3000.mol", __file__))
print(m.canonicalSmiles())
print(m.molfile())

print("****** S-group's SPL (PARENT) support ********")
m = indigo.loadMoleculeFromFile(dataPath("molecules/sgroups/sgroups-V2000.mol"))
indigo.setOption("molfile-saving-mode", "2000")
print(m.canonicalSmiles())
print(m.molfile())

indigo.setOption("molfile-saving-mode", "3000")
m = indigo.loadMoleculeFromFile(dataPath("molecules/sgroups/sgroups-V3000.mol"))
print(m.canonicalSmiles())
print(m.molfile())

print("****** Load custom collection ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/custom_collection.mol", __file__))
print(m.molfile())

print("****** Load TEMPLATE (SCSR) structure ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/SCSR_test.mol", __file__))
print(m.molfile())

print("****** Alias handling (V2000) ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/alias_marvin_v2000.mol", __file__))
indigo.setOption("molfile-saving-mode", "2000")
print(m.molfile())
print("****** Alias handling (V3000) ********")
indigo.setOption("molfile-saving-mode", "3000")
print(m.molfile())

print("****** Alias handling (CML) ********")
print(m.cml())

m = indigo.loadMoleculeFromFile(joinPathPy("molecules/alias_marvin.cml", __file__))
indigo.setOption("molfile-saving-mode", "2000")
print(m.molfile())

print("****** Test load from gzip buffer ********")
with open (joinPathPy("molecules/benzene.mol.gz", __file__), 'rb') as gz_mol:
    buf = gz_mol.read()
    if isIronPython():
        from System import Array, Byte
        buf_arr = bytearray(buf)
        buf = Array[Byte]([Byte(b) for b in buf_arr])

    m = indigo.loadMoleculeFromBuffer(buf)
    print(m.canonicalSmiles())

print("****** Load V3000 with DISP keyword ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/V3000_disp.mol", __file__))
indigo.setOption("molfile-saving-mode", "3000")
print(m.molfile())

print("****** Load V3000 with unknown keyword ********")
try:
   mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/V3000_unknown.mol", __file__))
except IndigoException as e:
   print(getIndigoExceptionText(e))

try:
   mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/V3000_unknown_atom_key.mol", __file__))
except IndigoException as e:
   print(getIndigoExceptionText(e))


print("****** Name is skeletal prefix ********")
try:
   m = indigo.loadMolecule("sil")      
except IndigoException as e:
   print(getIndigoExceptionText(e))
