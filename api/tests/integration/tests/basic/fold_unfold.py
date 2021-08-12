from __future__ import print_function
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

def testFoldUnfoldSDF (sdfile):
    print("testing " + relativePath(sdfile))
    indigo.setOption("treat-x-as-pseudoatom", "true")
    indigo.setOption("ignore-stereochemistry-errors", "true")
    for mol in indigo.iterateSDFile(sdfile):
        print(mol.smiles())
        smi1 = mol.canonicalSmiles()
        mol.unfoldHydrogens()
        print(mol.smiles())
        smi2 = mol.canonicalSmiles()
        mol.foldHydrogens()
        print(mol.smiles())
        smi3 = mol.canonicalSmiles()
        print()
        if smi1 != smi2 or smi2 != smi3:
            print("ERROR")
        
def testFoldUnfoldSingleMol (smiles):
    print("testing " + smiles)
    mol = indigo.loadMolecule(smiles)
    mol2 = mol.clone()
    mol2.unfoldHydrogens()
    print(mol2.countAtoms())
    mol2.foldHydrogens()
    print(mol2.countAtoms())
    
def testFoldUnfoldSingleReaction (smiles):
    print("testing " + smiles)
    rxn = indigo.loadReaction(smiles)
    rxn.foldHydrogens()
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn2 = rxn.clone()
    rxn.unfoldHydrogens()
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn2.unfoldHydrogens()
    for mol in rxn2.iterateMolecules():
        print(mol.countAtoms())
    rxn2.foldHydrogens()
    for mol in rxn2.iterateMolecules():
        print(mol.countAtoms())
      
testFoldUnfoldSDF(joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__))
testFoldUnfoldSingleMol("CC[H]")
testFoldUnfoldSingleReaction("[H]CC>>CC")
testFoldUnfoldSingleMol("[H][H]")
testFoldUnfoldSingleMol("[2H]C")
testFoldUnfoldSDF(joinPathPy("molecules/cis_trans_hydrogens_cycle.sdf", __file__))
