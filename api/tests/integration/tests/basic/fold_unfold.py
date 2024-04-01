from __future__ import print_function

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def testFoldUnfoldSDF(sdfile):
    print("testing " + relativePath(sdfile))
    indigo.setOption("treat-x-as-pseudoatom", "true")
    indigo.setOption("ignore-stereochemistry-errors", "true")
    for mol in indigo.iterateSDFile(sdfile):
        mol.clearXYZ()
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


def testAutoFoldUnfoldSingleMol(smiles):
    print("testing auto mode for " + smiles)
    mol = indigo.loadMolecule(smiles)
    mol2 = mol.clone()
    mol2.foldUnfoldHydrogens()
    print(mol2.countAtoms())
    mol2.foldUnfoldHydrogens()
    print(mol2.countAtoms())


def testAutoFoldUnfoldSingleReaction(smiles):
    print("testing auto mode for reaction " + smiles)
    rxn = indigo.loadReaction(smiles)
    rxn.foldUnfoldHydrogens()
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn2 = rxn.clone()
    rxn.foldUnfoldHydrogens()
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn2.foldUnfoldHydrogens()
    for mol in rxn2.iterateMolecules():
        print(mol.countAtoms())
    rxn2.foldUnfoldHydrogens()
    for mol in rxn2.iterateMolecules():
        print(mol.countAtoms())


def testFoldUnfoldSingleMol(smiles):
    print("testing " + smiles)
    mol = indigo.loadMolecule(smiles)
    mol2 = mol.clone()
    mol2.unfoldHydrogens()
    print(mol2.countAtoms())
    mol2.foldHydrogens()
    print(mol2.countAtoms())


def testFoldUnfoldSingleQueryMol(smiles):
    print("testing query " + smiles)
    mol = indigo.loadQueryMolecule(smiles)
    mol2 = mol.clone()
    mol2.unfoldHydrogens()
    print(mol2.countAtoms())
    mol2.foldHydrogens()
    print(mol2.countAtoms())


def testFoldUnfoldSMARTS(smarts):
    print("testing smarts " + smarts)
    mol = indigo.loadSmarts(smarts)
    mol.unfoldHydrogens()
    print(mol.countAtoms())
    mol.foldHydrogens()
    print(mol.countAtoms())


def testFoldUnfoldSingleReaction(smiles):
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


def testFoldUnfoldQueryReaction(smiles):
    print("testing query rection " + smiles)
    rxn = indigo.loadQueryReaction(smiles)
    rxn.foldHydrogens()
    print(rxn.smiles())
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn.unfoldHydrogens()
    print(rxn.smiles())
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())
    rxn.foldHydrogens()
    print(rxn.smiles())
    for mol in rxn.iterateMolecules():
        print(mol.countAtoms())


testFoldUnfoldSDF(
    joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__)
)
testFoldUnfoldSingleMol("CC[H]")
testFoldUnfoldSingleReaction("[H]CC>>CC")
testFoldUnfoldSingleMol("[H][H]")
testFoldUnfoldSingleMol("[2H]C")
testFoldUnfoldSDF(
    joinPathPy("molecules/cis_trans_hydrogens_cycle.sdf", __file__)
)
testFoldUnfoldSingleQueryMol("CC[H]")
testFoldUnfoldQueryReaction("[H]CC>>CC")
testFoldUnfoldSingleQueryMol("[H][H]")
testFoldUnfoldSingleQueryMol("[2H]C")
testFoldUnfoldSingleQueryMol(
    "N#CC(C#N)=C1C([H])=C([H])C(=C(C#N)C#N)C([H])=C1[H] |t:4,10|"
)

testFoldUnfoldSingleQueryMol("c1ccccc1")
testFoldUnfoldSingleQueryMol("CCC")
testFoldUnfoldSMARTS("c1ccccc1")
testFoldUnfoldSMARTS("CCC")
testFoldUnfoldSMARTS("C-C-C")

mol = """
  Bond Single or Double   1252422 22D 1   1.00000     0.00000     0

  2  1  0  0  0  0  0  0  0  0999 V2000
    2.9920   -1.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8580   -0.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  5  0     0  0
M  END
"""
testFoldUnfoldSingleQueryMol(mol)
mol = """
  Bond Single or Aromatic   1252422 22D 1   1.00000     0.00000     0

  2  1  0  0  0  0  0  0  0  0999 V2000
    2.9920   -1.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8580   -0.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  6  0     0  0
M  END
"""
testFoldUnfoldSingleQueryMol(mol)
mol = """
  Bond Double or Aromatic  1252422 22D 1   1.00000     0.00000     0

  2  1  0  0  0  0  0  0  0  0999 V2000
    2.9920   -1.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8580   -0.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  7  0     0  0
M  END
"""
testFoldUnfoldSingleQueryMol(mol)
mol = """
  Bond Any  1252422 22D 1   1.00000     0.00000     0

  2  1  0  0  0  0  0  0  0  0999 V2000
    2.9920   -1.4500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8580   -0.9500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  8  0     0  0
M  END
"""
testFoldUnfoldSingleQueryMol(mol)

testAutoFoldUnfoldSingleMol("CC[H]")
testAutoFoldUnfoldSingleReaction("[H]CC>>CC")
testAutoFoldUnfoldSingleMol("[H][H]")
testAutoFoldUnfoldSingleMol("[2H]C")
