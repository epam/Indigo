import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

def printInfo(mol):
    print("Molecule: %s" % mol.name())
    print("Atom block:")
    with mol.iterateAtoms() as iterator:
        while iterator.hasNext() :
            atom = iterator.next()
            print('%s, isotope: %d, charge: %d' % (atom.symbol(), atom.isotope(), atom.charge()))
    print("Bond block:")
    with mol.iterateBonds() as iterator:
        while iterator.hasNext() :
            bond = iterator.next()
            print('order: %d, stereo: %d' % (bond.bondOrder(), bond.bondStereo()))

def testMolv2000Charge(filename):

    print(filename)
    mol = indigo.loadMoleculeFromFile(joinPathPy(filename, __file__))
    printInfo(mol)

def testSDF2000Charge(filename):

    print(filename)
    with indigo.iterateSDFile(joinPathPy(filename, __file__)) as iterator:
        while iterator.hasNext():
            printInfo(iterator.next())

try:
    testMolv2000Charge("molecules/test_molv2000_charge.mol")
    testSDF2000Charge("molecules/public-structures-bingo-parse-errors.sdf")
except IndigoException as e:
    print("caught " + getIndigoExceptionText(e))
