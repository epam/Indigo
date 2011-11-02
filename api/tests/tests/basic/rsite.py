import os;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1");
if not os.path.exists("out"):
   os.makedirs("out")
saver = indigo.createFileSaver("out/rsite.sdf", "sdf")
mol = indigo.loadMolecule("CCNNCN");
mol.addRSite("R");
mol.addRSite("R");
mol.addRSite("R1");
mol.addRSite("");
a3 = mol.addRSite("R3");
print(mol.molfile())
saver.append(mol)
mol.addRSite("R1, R3");
print(mol.molfile())
saver.append(mol)
a3.resetAtom("N")
print(mol.molfile())
saver.append(mol)
a0 = mol.getAtom(0)
a0.setRSite("R4")
print(mol.molfile())
saver.append(mol)
a1 = mol.getAtom(1)
a1.resetAtom("O")
print(mol.molfile())
saver.append(mol)
a1.setRSite("R4")
a1.highlight()
print(mol.molfile())
saver.append(mol)
