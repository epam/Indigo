import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def testAutomap1():
    print("*** Simple test 1 ***")
    rxn1 = indigo.loadReaction(
        "CC1=CSC(C=C(N=[N+]=[N-])C(OCC)=O)=C1>>CC1=CSC2=C1NC(C(OCC)=O)=C2"
    )
    rxn1.automap("discard")
    print(rxn1.smiles())


def testAutomap2():
    print("*** Simple test 2 ***")
    rxn = indigo.createReaction()
    rxn.addProduct(indigo.loadMolecule("C(CC1=CC=CC=C1)C1=CC=C(S1)C=O"))
    rxn.addReactant(indigo.loadMolecule("C1(=CC=CC=C1)C#CC1=CC=C(S1)C=O"))
    rxn.addReactant(indigo.loadMolecule("C(CC1=CC=CC=C1)C=1C=C(SC1)C=O"))
    rxn.automap("discard")
    print(rxn.smiles())


def testAutomap3():
    print("*** Test reaction with single atom component mapping ***")
    rxn = indigo.createReaction()
    rxn.addReactant(indigo.loadMolecule("CC(OC)=O"))
    rxn.addReactant(indigo.loadMolecule("[Na+].[OH-]"))
    rxn.addProduct(indigo.loadMolecule("CC(O)=O"))
    rxn.addProduct(indigo.loadMolecule("C[O-].[Na+]"))
    rxn.automap("discard")
    print(rxn.smiles())


def testAutomap4():
    print("*** Testing foldHydrogens with automap ***")
    reaction = indigo.createReaction()
    reaction.addProduct(
        indigo.loadMolecule("O=C1CCC[C@]2([H])C3C=CC(C3)[C@@]21[H]")
    )
    reaction.addReactant(indigo.loadMolecule("C1C=CC=C1"))
    reaction.addReactant(indigo.loadMolecule("O=C1CCCC=C1"))
    print(reaction.smiles())
    reaction.foldHydrogens()  # if folding is turned on then there is an a error in automap
    print(reaction.smiles())
    reaction.automap("discard")
    print(reaction.smiles())


def testAutomap5():
    print("*** Test reaction with single atoms***")
    rxn = indigo.createReaction()
    rxn.addReactant(indigo.loadMolecule("CC(OC)=O"))
    rxn.addReactant(indigo.loadMolecule("[Na+]"))
    rxn.addProduct(indigo.loadMolecule("CC(O)=O"))
    rxn.addProduct(indigo.loadMolecule("[Na+]"))
    rxn.automap("discard")
    print(rxn.smiles())


def testAutomap6():
    print("*** Test query reaction ***")
    qr = indigo.loadQueryReaction("C1=CC=CC=C1>>C1=CC=CC(CC2=CC=CC=C2)=C1")
    qr.automap("discard")
    print(qr.smiles())


def testAutomap(testname, reactants, products, mode="discard"):
    print("*** %s ***" % testname)
    rxn = indigo.createReaction()
    for r in reactants:
        rxn.addReactant(indigo.loadMolecule(r))
    for p in products:
        rxn.addProduct(indigo.loadMolecule(p))
    print("Before:")
    print(rxn.smiles())
    rxn.automap(mode)
    print("After:")
    print(rxn.smiles())


def testAutomapR(testname, rxn_s, mode="discard"):
    print("*** %s ***" % testname)
    rxn = indigo.loadReaction(rxn_s)
    print("Before:")
    print(rxn.canonicalSmiles())
    rxn.automap(mode)
    print("After:")
    print(rxn.canonicalSmiles())


def testAutomapQuery(testname, reactants, products, mode="discard"):
    print("*** %s ***" % testname)
    rxn = indigo.createQueryReaction()
    for r in reactants:
        rxn.addReactant(indigo.loadQueryMolecule(r))
    for p in products:
        rxn.addProduct(indigo.loadQueryMolecule(p))
    print("Before:")
    print(rxn.smiles())
    rxn.automap(mode)
    print("After:")
    print(rxn.smiles())


testAutomap1()
testAutomap2()
testAutomap3()
testAutomap4()
testAutomap5()
testAutomap(
    "AAM1",
    ["O=CN(C)C", "S(=O)(Cl)Cl", "OC(=O)c1ccccn1", "[OH-].[Na+]"],
    ["Cl.C(C)OC(=O)C1=NC=CC(=C1)Cl"],
)
testAutomap(
    "AAM2",
    ["S(=O)(Cl)Cl", "OC(=O)c1ccccn1", "[OH-].[Na+]", "[NH4+].[OH-]"],
    ["ClC=1C=C(N=CC1)C(=O)N"],
)
testAutomap(
    "AAM3",
    ["[H-].[Na+]", "N#Cc1ccccn1", "NC(N)=O", "S(O)(O)(=O)=O"],
    ["N1=C(C=CC=C1)C1=NC(=NC(=N1)C1=NC=CC=C1)O"],
)
testAutomap(
    "AAM4",
    ["[H-].[Na+]", "N#Cc1ccccn1", "Cl.NC(=N)N"],
    ["N1=C(C=CC=C1)C1=NC(=NC(=N1)C1=NC=CC=C1)N"],
)
testAutomap("AAM5", ["C1CC1"], ["C1CCC1C"])
testAutomap("AAM6", ["C1CCC1C"], ["C1CC1"])
testAutomap(
    "D-Y-exchange1",
    ["C1CC1", "C1CC1", "C1CC1"],
    ["CC(C)C", "CC(C)C", "CC(C)C"],
)
testAutomap("D-Y-exchange2", ["C1CC1", "CC(C)C"], ["CC(C)C", "C1CC1"])
testAutomap("D-Y-exchange3", ["C1C2C11C3CC213"], ["CC(C)C(C)C(C)C(C)C"])
testAutomap("CON1", ["CCCC", "NNNN.OOOO"], ["CCCC.NNNN", "OOOO"])
testAutomap("CON2", ["CCCC", "NNNN.NNNN"], ["CCCC", "NNNN"])
testAutomap("INGNORE CHARGES0", ["CCCCC"], ["CC[C++]CC"])
testAutomap(
    "INGNORE CHARGES1", ["CCCCC"], ["CC[C++]CC"], "discard ignore_charges"
)
testAutomap("INGNORE ISOTOPE0", ["CCCCC"], ["CC[8CH2]CC"])
testAutomap(
    "INGNORE ISOTOPE1", ["CCCCC"], ["CC[8CH2]CC"], "discard ignore_isotopes"
)
testAutomap("INGNORE VALENCE0", ["CC[GeH2]CC"], ["CC[Ge]CC"])
testAutomap(
    "INGNORE VALENCE1", ["CC[GeH2]CC"], ["CC[Ge]CC"], "discard ignore_valence"
)
testAutomap("INGNORE RADICAL0", ["CCCCC"], ["CC[CH]CC"])
testAutomap(
    "INGNORE RADICAL1", ["CCCCC"], ["CC[CH]CC"], "discard ignore_radicals"
)
testAutomap(
    "AUTOMORPHISM KEEP ATOM DEGREE",
    ["BrC1=CC=CC=C1"],
    ["C1=CC=CC=C1C1CN(CCN1)C"],
    "discard",
)
testAutomap(
    "AUTOMORPHISM KEEP BOND ORDER",
    ["C=C1CC=C(C=C1)P(C1=CC=CC=C1)C1=CC=CC=C1"],
    ["C(NC1C=CC=C1)C1CC=C(C=C1)P(C1=CC=CC=C1)C1=CC=CC=C1"],
    "discard",
)
testAutomap(
    "AUTOMORPHISM KEEP ATOM NUMBER",
    ["COC(C1=C(C=CC(=C1)I)O)=O", "C(C)C(C(=O)O)Br"],
    ["COC(C1=C(C=CC(=C1)I)OCC(=O)OCC)=O"],
    "discard",
)
testAutomap(
    "AAM WITH DISSOCIATIONS",
    [
        "NCC(c1c2c(ccc(OC)c2)ccc1)CO",
        "CCN(CC)CC",
        "FC(F)(F)C(OC(=O)C(F)(F)F)=O",
    ],
    ["FC(F)(F)C(NCC(c1c2c(ccc(OC)c2)ccc1)CO)=O"],
    "discard",
)
testAutomapQuery(
    "QUERY AAM", ["C1=CC=CC=C1"], ["C1=CC=CC(CC2=CC=CC=C2)=C1"], "discard"
)
testAutomapR(
    "Keep mapping in KEEP mode",
    "C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1",
    "KEEP",
)
testAutomapR(
    "Keep mapping in ALTER mode",
    "C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1",
    "ALTER",
)
testAutomapR(
    "Keep mapping in KEEP mode for radicals",
    "C[12CH2:1]C(CCCC)[CH]CCCCCCC>>C[13CH2:1]C(CCCC)[C]CCCCCCCC |^1:7,^4:22|",
    "KEEP",
)
