import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

def testReactionInstrumentation ():
    rxn = indigo.loadReaction("[PH5].CN>CC>")
    rxn.addReactant(indigo.loadMolecule("Oc1ccccc1"))
    rxn.addProduct(indigo.loadMolecule("n1ccccc1"))
    cat = indigo.createMolecule()
    cat.addAtom("2.  acid")
    rxn.addCatalyst(cat)
    print(rxn.smiles())
    print(rxn.rxnfile())
    print("{0} reactants".format(rxn.countReactants()))
    print("{0} catalysts".format(rxn.countCatalysts()))
    print("{0} products".format(rxn.countProducts()))
    print("{0} molecules".format(rxn.countMolecules()))
    for mol in rxn.iterateMolecules():
        print("MOLECULE: " + mol.canonicalSmiles())
    for mol in rxn.iterateReactants():
        print("REACTANT: " + mol.canonicalSmiles())
    for mol in rxn.iterateCatalysts():
        print("CATALYST: " + mol.canonicalSmiles())
    for mol in rxn.iterateProducts():
        print("PRODUCT:  " + mol.canonicalSmiles())
    print("\nREMOVING")
    rxn.iterateReactants().next().remove()
    rxn.iterateProducts().next().remove()
    print("{0} reactants".format(rxn.countReactants()))
    print("{0} catalysts".format(rxn.countCatalysts()))
    print("{0} products".format(rxn.countProducts()))
    print("{0} molecules".format(rxn.countMolecules()))
    for mol in rxn.iterateMolecules():
        print("MOLECULE: " + mol.canonicalSmiles())
    for mol in rxn.iterateReactants():
        print("REACTANT: " + mol.canonicalSmiles())
    for mol in rxn.iterateCatalysts():
        print("CATALYST: " + mol.canonicalSmiles())
    for mol in rxn.iterateProducts():
        print("PRODUCT:  " + mol.canonicalSmiles())
      
testReactionInstrumentation()
