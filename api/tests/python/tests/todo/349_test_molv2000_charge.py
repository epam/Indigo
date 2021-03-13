import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

    
def testMolv2000Charge(filename):
    
    print(relativePath(filename))
    try:
        mol = indigo.loadMoleculeFromFile(joinPath(filename))
        print("loaded.")
    except IndigoException as e:
        print("caught " + getIndigoExceptionText(e))

testMolv2000Charge("molecules/test_molv2000_charge.mol")
testMolv2000Charge("molecules/public-structures-bingo-parse-errors.sdf")
