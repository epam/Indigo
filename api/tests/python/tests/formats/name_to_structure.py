import os
import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()


def printTestName(name):
    print("TEST: {0}".format(name))


def namesToSmiles(names):
    try:
        for name in names:
            mol = indigo.nameToStructure(name)
            print("Name: {0} \n\tSMILES: {1}".format(name, mol.canonicalSmiles()))
    except IndigoException as e:
        print("ERROR: name {0}; error {1}".format(name, getIndigoExceptionText(e)))


def testRetainedNames():
    printTestName("retained alkanes")
    names = ["methane", "ethane", "propane", "butane"]
    namesToSmiles(names)
    print("DONE\n")

    printTestName("retained alkenes")
    names = ["ethene", "propene", "butene"]
    namesToSmiles(names)
    print("DONE\n")

    printTestName("retained alkynes")
    names = ["ethyne", "propyne", "butyne"]
    namesToSmiles(names)
    print("DONE\n")


def testAcyclicAlkanes():
    printTestName("acyclic alkanes")
    names = ["octane", "octene", "octyne"]
    namesToSmiles(names)
    print("DONE\n")


def testLocants():
    printTestName("locants")
    names = ["oct-3-ene", "oct-5,3-diene", "oct-3-yne", "oct-3,5-diyne", "3-ethyl-octane", "3,5-diethyl-octane",
             "3-methyl-5-ethyl-octane", "3-(2,4-dimethyl-pentyl)-octane"]
    namesToSmiles(names)
    print("DONE\n")


def testErrors():
    printTestName("errors")
    names = ["", "random text", "1234"]
    namesToSmiles(names)
    print("DONE\n")


def testMoleculeLoader():
    printTestName("loadMolecule")
    names = ["methane", "ethane", "propane", "butane", "ethene", "propene", "butene", "ethyne", "propyne", "butyne",
             "oct-3-ene", "oct-5,3-diene", "oct-3-yne", "oct-3,5-diyne", "3-ethyl-octane", "3,5-diethyl-octane",
             "3-methyl-5-ethyl-octane", "3-(2,4-dimethyl-pentyl)-octane", "cyclooctane", "cyclooctene", "cyclooctyne",
             "3-methyl-5-ethyl-cyclooctane", "cyclotetradecane", "cyclododeca-1,3,5,7,9,11-hexaene"]
    try:
        for name in names:
            mol = indigo.loadMolecule(name)
            print("Name {0} opened with loadMolecule\n\tSMILES {1}".format(name, mol.canonicalSmiles()))
    except IndigoException as e:
        print("ERROR: name {0}; error {1}".format(name, getIndigoExceptionText(e)))
    print("DONE\n")


def testCyclic():
    printTestName("cyclic")
    names = ["cyclooctane", "cyclooctene", "cyclooctyne", "3-methyl-5-ethyl-cyclooctane", "cyclotetradecane",
             "cyclododeca-1,3,5,7,9,11-hexaene", "1,3-dimethylcyclohexane", "1,2-dimethylcyclohexane",
             "1,1-dimethylcyclohexane"]
    namesToSmiles(names)
    print("DONE\n")


def testRelaxedIUPAC():
    printTestName("relaxed IUPAC grammar")
    names = ["2-octene", "oct-2-ene", "3,5-octadiene", "octa-3,5-ene", "2,4,6-octatriyn", "octa-2,4,6-triyn",
             "1,3-octadiene-5,7-diyn", "octa-1,3-diene-5,7-diyn"]
    namesToSmiles(names)
    print("DONE\n")


def testSkeletal():
    printTestName("skeletal 'a' substitution")
    names = ["silane", "bromane", "silapentane", "2-silapropane", "6-sila-2,4-octadiene-6-yn", "2,4,6,8-tetrasila-5-ethyl-decane",
             "6-sila-octa-2,4-diene-6-yn", "6-sila-2,4-octadiene-6-yn", "1,7-disila-3,5-dithia-2,4-octadiene",
             "2-silaprop-2-ene", "silapropyne", "2-silaprop-2-yne", "8-thia-2,4,6-trisiladecane", "2-oxa-4,6,8-trisilanonane",
             "2,4,6,8-tetrasiladec-9-ene"]
    namesToSmiles(names)
    print("DONE\n")


def testTrivial():
    printTestName("trivial names")
    names = ["allene", "xylol", "acenaphthylene", "picene", "pyridine", "carbazol", "capric acid", "pimelic acid",
             "aconitic acid", "glutamine"]
    namesToSmiles(names)
    print("DONE\n")


testRetainedNames()
testAcyclicAlkanes()
testLocants()
testCyclic()
testMoleculeLoader()
testRelaxedIUPAC()
testSkeletal()
testTrivial()
#testErrors()