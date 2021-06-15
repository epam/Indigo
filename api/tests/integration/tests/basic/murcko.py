import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
  
def calculateMurckoScaffold (mol):
    natoms = mol.countAtoms() + 1
    while natoms > mol.countAtoms():
        natoms = mol.countAtoms()
        hanging_atoms = []
         
        for atom in mol.iterateAtoms():
            if atom.degree() <= 1:
                hanging_atoms.append(atom)
         
        while len(hanging_atoms) > 0:
            to_remove = []
            hanging_next = []
            
            for atom in hanging_atoms:
                if atom.degree() == 0:
                    to_remove.append(atom)
                else:
                    nei = atom.iterateNeighbors().next()
                  
                    if nei.degree() <= 2 or nei.bond().bondOrder() == 1:
                        to_remove.append(atom);
                    elif atom not in hanging_next:
                        hanging_next.append(atom);

            for atom in to_remove:
                if atom.degree() > 0:
                    nei = atom.iterateNeighbors().next()
                  
                    if nei.degree() == 2:
                        found = False
                     
                        for a in to_remove:
                            if a.index() == nei.index():
                                found = True
                                break
                         
                        if not found:
                            for a in hanging_next:
                                if a.index() == nei.index():
                                    found = True
                                    break
                                    
                            if not found:
                                hanging_next.append(nei)
            
            if len(to_remove) == 0:
                break
            
            for atom in to_remove:
                atom.remove()
            
            hanging_atoms = hanging_next
            
saver = indigo.createFileSaver(joinPath("out/murcko.sdf"), "sdf")

m = indigo.loadMolecule("CCCCCC1CCCCC1")
calculateMurckoScaffold(m)
print(m.smiles())
saver.append(m)

for m in indigo.iterateSDFile(joinPath("molecules/murcko.sdf")):
    m2 = m.clone()
    calculateMurckoScaffold(m2)
    print(m2.smiles())
    saver.append(m2)
    buf = m2.serialize()
    m3 = indigo.unserialize(buf)
    print(m3.smiles())
    saver.append(m3)
    
    