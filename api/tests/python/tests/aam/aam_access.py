import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
reactions = [
   "CC1=CSC(C=C(N=[N+]=[N-])C(OCC)=O)=C1>>CC1=CSC2=C1NC(C(OCC)=O)=C2",
   [ ["C1(=CC=CC=C1)C#CC1=CC=C(S1)C=O", "C(CC1=CC=CC=C1)C=1C=C(SC1)C=O"], ["C(CC1=CC=CC=C1)C1=CC=C(S1)C=O"] ],
   [ ["CC(OC)=O",  "[Na+].[OH-]" ], ["CC(O)=O", "C[O-].[Na+]" ] ],
   "CCC>>CCO",
]
def loadReaction (item):
    if isinstance(item, str):
        return indigo.loadReaction(item)
    
    # create reaction by molecules
    rxn = indigo.createReaction()
    for react in item[0]:
        rxn.addReactant(indigo.loadMolecule(react))
        
    for prod in item[1]:
        rxn.addProduct(indigo.loadMolecule(prod))
    return rxn

def printAAM (rxn):
    print(rxn.smiles())
    for mol in rxn.iterateMolecules():
        print("  Mol: %s" % (mol.smiles()))
        for a in mol.iterateAtoms():
            aamNumber = rxn.atomMappingNumber(a)
            if aamNumber:
                print("%d, %d: %d" % (a.index(), a.atomicNumber(), aamNumber))
   
def testAutomap (item):
    rxn = loadReaction(item)
    print(rxn.smiles())
    rxn.automap("discard")
    printAAM(rxn)
    print("Clear AAM and fix first atoms")
    rxn.clearAAM()
    r1 = rxn.iterateReactants().next()
    p1 = rxn.iterateProducts().next()
    ra1 = r1.iterateAtoms().next()
    pa1 = p1.iterateAtoms().next()
    # fix first atoms
    rxn.setAtomMappingNumber(ra1, 1)
    rxn.setAtomMappingNumber(pa1, 1)
    printAAM(rxn)
    print("Remapping with fixed atoms")
    rxn.automap("keep")
    printAAM(rxn)

def main():
    for item, number in zip(reactions, range(1000)):
        print("\n*** Test %d ***" % number)
        testAutomap(item)

if __name__ == '__main__':
    main()
