import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

def testSmarts (m):
    try:
       print(m.smarts())
    except IndigoException as e:
       print(getIndigoExceptionText(e))
       print(m.smiles())


molstr = '''
  Ketcher 11241617102D 1   1.00000     0.00000     0

  8  7  0     0  0            999 V2000
    3.7000   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5660   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.4321   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.2981   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.1641   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0301   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.8962   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.7622   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  7  8  1  0     0  0
M  CHG  1   3   5
M  END
'''

print("**** Load and Save as Query ****")
m = indigo.loadQueryMolecule(molstr)
testSmarts(m)

print("**** Load and Save as Molecule ****")
m = indigo.loadMolecule(molstr)
testSmarts(m)

