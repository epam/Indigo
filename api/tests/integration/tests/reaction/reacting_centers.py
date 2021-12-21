import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def test1():
    print("*** Test 1 ***")
    rxn = indigo.createReaction()
    rxn.addProduct(indigo.loadMolecule("CCCC"))
    rxn.addReactant(indigo.loadMolecule("CCCC"))
    print(rxn.smiles())
    print("reacting centers:")
    for m in rxn.iterateMolecules():
        for b in m.iterateBonds():
            print(int(rxn.reactingCenter(b)))
    for m in rxn.iterateMolecules():
        for b in m.iterateBonds():
            if not isIronPython():
                rxn.setReactingCenter(
                    b, Indigo.RC_CENTER | Indigo.RC_UNCHANGED
                )
            else:
                rxn.setReactingCenter(
                    b, ReactingCenter.CENTER | ReactingCenter.UNCHANGED
                )
    print("modified centers:")
    for m in rxn.iterateMolecules():
        for b in m.iterateBonds():
            print(int(rxn.reactingCenter(b)))


def test2():
    print("*** Test AAM ***")
    rxn2 = indigo.loadReaction("CC=O>>CCO")
    print("reaction smiles " + rxn2.smiles())
    for m in rxn2.iterateMolecules():
        for b in m.iterateBonds():
            if not isIronPython():
                rxn2.setReactingCenter(
                    b, Indigo.RC_UNCHANGED | Indigo.RC_ORDER_CHANGED
                )
            else:
                rxn2.setReactingCenter(
                    b, ReactingCenter.UNCHANGED | ReactingCenter.ORDER_CHANGED
                )
    rxn2.automap("DISCARD")
    print("aam reaction smiles for given RC " + rxn2.smiles())


def test3():
    print("*** Test correct reacting centers ***")
    rxn = indigo.loadReaction(
        "[CH3:7][CH2:6][CH2:1][CH2:2]O[CH2:4][CH2:5][CH2:8][CH3:9]>>[CH3:7][CH2:6][CH:1]=[CH:2]C[CH2:4][CH2:5][C:8]#[CH:9]"
    )
    print("reacting centers:")
    for m in rxn.iterateMolecules():
        for b in m.iterateBonds():
            print(int(rxn.reactingCenter(b)))
    rxn.correctReactingCenters()
    print("modified centers:")
    for m in rxn.iterateMolecules():
        for b in m.iterateBonds():
            print(int(rxn.reactingCenter(b)))
    # print(rxn.rxnfile())


test1()
test2()
test3()
