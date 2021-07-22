import os
import sys
sys.path.append('../../common')
from env_indigo import *

testName = "Load Molecule Structure"
print("******  Test: (" + testName + ") ***")

indigo = Indigo()
testNo = 1

def testLoadFromBuffer():
    global testNo
    print("\n---- TestLoadFromBuffer ----")
    
    try:
        fileName = "../../../../../data/molecules/affine/2-bromobenzenethiol-rot.mol"
        with open(joinPath(fileName), "rb") as binaryFile:
            print("\n(** #{0} **): no parameter".format(testNo))
            testNo += 1
            data = binaryFile.read()
            if isIronPython():
                from System import Array, Byte
                buf_arr = bytearray(data)
                data = Array[Byte]([Byte(b) for b in buf_arr])

            m = indigo.loadStructureFromBuffer(data)
            print(m.canonicalSmiles())
            print(m.smiles())
            print("Atoms: {0}".format(m.countAtoms()))
            print("Bonds: {0}".format(m.countBonds()))
    
            print("\n(** #{0} **): Load as query".format(testNo))
            testNo += 1
            m = indigo.loadStructureFromBuffer(data, "query")
            print("Atoms: {0}".format(m.countAtoms()))
            print("Bonds: {0}".format(m.countBonds()))
    
            print("\n(** #{0} **): Load as smarts".format(testNo))
            testNo += 1
            m = indigo.loadStructureFromBuffer(data, "smarts")
            print("Atoms: {0}".format(m.countAtoms()))
            print("Bonds: {0}".format(m.countBonds()))
    except IndigoException as e:
        print(e)
        
                
    try:
        fileName = "../../../../../data/molecules/basic/benzodiazepine.sdf.gz"
        with open(joinPath(fileName), "rb") as binaryFile:
            print("\n(** #{} **): Load from compessed binary ({})".format(testNo, fileName))
            testNo += 1
            data = binaryFile.read()

            if isIronPython():
                from System import Array, Byte
                buf_arr = bytearray(data)
                data = Array[Byte]([Byte(b) for b in buf_arr])
                
            m = indigo.loadStructureFromBuffer(data)
            print("Atoms: {0}".format(m.countAtoms()))
            print("Bonds: {0}".format(m.countBonds()))
    except IndigoException as e:
        print(e)


def testLoadFromFile():
    global testNo
    print("\n---- TestLoadFromFile ----")
    
    fileName = "../../../../../data/molecules/affine/2-bromobenzenethiol-rot.mol"
    
    try:    
        print("\n(** #{0} **): Load from file ({1})".format(testNo, fileName))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName))
        print(m.canonicalSmiles())
        print(m.smiles())
        print("Atoms: {0}".format(m.countAtoms()))
        print("Bonds: {0}".format(m.countBonds()))
    
        print("\n(** #{0} **): Load as query".format(testNo))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName), "query")
        print("Atoms: {0}".format(m.countAtoms()))
        print("Bonds: {0}".format(m.countBonds()))
    
        print("\n(** #{0} **): Load as smarts".format(testNo))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName), "smarts")
        print("Atoms: {0}".format(m.countAtoms()))
        print("Bonds: {0}".format(m.countBonds()))
    except IndigoException as e:
        print(e)

    try:
        fileName = "molecules/large-symmetric.smi"
        
        print("\n(** #{0} **): Load smiles from file ({1})".format(testNo, fileName))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName))
        print("Atoms: {0}".format(m.countAtoms()))
        print("Bonds: {0}".format(m.countBonds()))    
    except IndigoException as e:
        print(e)
        
    try:
        fileName = "../../../../../data/reactions/basic/adama_reaction.rxn"
        
        print("\n(** #{0} **): Load reaction from file ({1})".format(testNo, fileName))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName))
        
        print("Molecules : {0}".format(m.countMolecules()))
        print("Reactants: {0}".format(m.countReactants()))
        print("Products : {0}".format(m.countProducts()))
        print("Catalysts: {0}".format(m.countCatalysts()))
    except IndigoException as e:
        print(e)

    try:
        fileName = "../../../../../data/molecules/basic/benzene.mol.gz"

        print("\n(** #{0} **): Load zipped file ({1})".format(testNo, fileName))
        testNo += 1
        m = indigo.loadStructureFromFile(joinPath(fileName))
        print("Atoms: {0}".format(m.countAtoms()))
        print("Bonds: {0}".format(m.countBonds()))    
    except IndigoException as e:
        print(e)


def testLoadFromString():
    global testNo
    try:
        print("\n---- SMILE Structure ----")
        print("\n(** #{0} **): Empty input:".format(testNo))
        testNo += 1
        c = ""
        m = indigo.loadStructure(c)
        print(m.smiles())
     
        c = "C1=C(*)C=CC=C1"
     
        print("\n(** #{0} **): Smile (no parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c)
        print(m.smiles())
         
        print("\n(** #{0} **): Smile (query parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "query")
        print(m.smiles())
         
        print("\n(** #{0} **): Smile (smarts parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "smarts")
        print(m.smiles())
        
        print("\n---- Reaction Structure ----")
        c = "C1=C(*)C=CC=C1>>C1=CC=CC(*)=C1"
    
        print("\n(** #{0} **): Reaction (query parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "query")
        print(m.smiles())
    
        print("\n(** #{0} **): smarts parameter:".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "smarts")
        print(m.smiles())
    
        c = "C1=C(*)C=CC=C1"    

        print("\n(** #{0} **): Smiles, (no parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c)
        print(m.smiles())
         
     
        c = "[OH]c1ccccc1"    
        print("\n(** #{0} **): Smarts, (smarts parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "smarts")
        print(m.smarts())
         
        print("\n(** #{0} **): Smarts, (query parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c, "query")
        print(m.smiles())
    
        print("\n(** #{0} **): Smarts, (no parameter):".format(testNo))
        testNo += 1
        m = indigo.loadStructure(c)
        print(m.smiles())
    
    except IndigoException as e:
        print(e)
    

try:
    
    testLoadFromString()
    testLoadFromBuffer()
    testLoadFromFile()
    
except IndigoException as e:
    print(e)

print("\n*** End Test: (" + testName + ")  ***")
