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
for item in indigo.iterateSDFile(joinPath("molecules/multiline_properties.sdf")):
    for prop in item.iterateProperties():
        print(prop.name() + " : " + prop.rawData())

print("****** CurlySMILES ********")
m = indigo.loadMolecule("PC{-}{+n}N")      
print(m.smiles())
m = indigo.loadMolecule("PC{-}O{+n}N")      
print(m.smiles())
