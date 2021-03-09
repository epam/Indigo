import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

#
# Prepare a molecule for printing out
#
def prepareStructure(mol):
   for atom in mol.iterateAtoms():
      atom.setXYZ(0, 0, 0)
   for rg in mol.iterateRGroups():
      if rg.iterateRGroupFragments().hasNext():
         rg_next = rg.iterateRGroupFragments().next()
         for atom in rg_next.iterateAtoms():
            atom.setXYZ(0, 0, 0)

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)


def printMolfile(mol):
    smiles = mol.canonicalSmiles()
    print("Smiles: " + smiles)
    #for format in ["2000", "3000", "auto"]:
    for format in ["auto"]:
        print("Format: " + format)
        indigo.setOption("molfile-saving-mode", format)
        molfile = mol.molfile()
        print(molfile)
        # Check correctness by loading molfile and saving into smiles
        sm2 = indigo.loadMolecule(molfile).canonicalSmiles()
        if smiles != sm2:
            sys.stderr.write("Error: smiles are different\n  %s\n  %s\n" % (smiles, sm2))
    indigo.setOption("molfile-saving-mode", "auto")

def printQMolfile(qmol):
    smiles = qmol.smiles()
    print("Smiles: " + smiles)
#    for format in ["2000", "3000", "auto"]:
    for format in ["auto"]:
        print("Format: " + format)
        indigo.setOption("molfile-saving-mode", format)
        molfile = qmol.molfile()
        print(molfile)
    indigo.setOption("molfile-saving-mode", "auto")
    
def printRGroups(mol):
   print("separate RGroups: ")
   for rg in mol.iterateRGroups():
      print("RGROUP # %d" % (rg.index()))
      if rg.iterateRGroupFragments().hasNext():
         print(rg.iterateRGroupFragments().next().molfile())
      else:
         print("NO FRAGMENT")


def testDecoIterate (scaffold, structures):
   deco = indigo.createDecomposer(scaffold)
   for smiles in structures:
      str = indigo.loadMolecule(smiles)
      item = deco.decomposeMolecule(str)
      match_idx = 1
      min_r = 100
      selected_match=None
      # loop over all the matches
      for q_match in item.iterateDecompositions():
         print("MATCH # %d" % match_idx)
         print("highlighted structure: " + q_match.decomposedMoleculeHighlighted().smiles())
         print("molecule with rgroups: ")
         rg_mol = q_match.decomposedMoleculeWithRGroups()
         prepareStructure(rg_mol)
         printMolfile(rg_mol)
         printRGroups(rg_mol)
         match_idx += 1
         # search match with minimum RGroup count
         if rg_mol.countRSites() < min_r:
            min_r=rg_mol.countRSites()
            selected_match=q_match
      # add selected match to the full scaffold
      deco.addDecomposition(selected_match)

   full_scaf = deco.decomposedMoleculeScaffold()
   prepareStructure(full_scaf)
   print("full scaffold: ")
   printQMolfile(full_scaf)

def testDecoIterateSmile (scaf, structures):
   scaffold = indigo.loadQueryMolecule(scaf)
   testDecoIterate (scaffold, structures)

def testDecoIterateFile (scaf, structures):
   scaffold = indigo.loadQueryMoleculeFromFile(scaf)
   testDecoIterate (scaffold, structures)

print("should add correct disconnected match to the full scaffold*************************************************************************")
testDecoIterateSmile("CCCNC.OCCCC", ["CCCCOCCCCCNCCC"])


