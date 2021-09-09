import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

for root, dirnames, filenames in os.walk(joinPathPy("molecules/set1", __file__)):
    filenames.sort()
    for filename in filenames:
        sys.stdout.write("%s: " % filename)        
        try:
            mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))        

print("****Invalid structures****")

def getIndigo ():
    indigo = Indigo()
    indigo.setOption("ignore-stereochemistry-errors", "true")
    indigo.setOption("ignore-noncritical-query-features", "true")
    return indigo

with open(joinPathPy("molecules/invalid.csv", __file__)) as f:
    cells = f.read().split(",")
    for idx, c in enumerate(cells):
        print("** %d **" % idx)
        try:
            indigo = getIndigo()
            m = indigo.loadMolecule(c)
            print(m.smiles())
        except IndigoException as e:
            print(getIndigoExceptionText(e))
