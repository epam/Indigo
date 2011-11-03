import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
if not os.path.exists("out"):
   os.makedirs("out")
saver = indigo.createFileSaver("out/sgroups-instrumentation.sdf", "sdf")
   
def testSGroupsInstrumentation ():
  indigo.setOption("molfile-saving-skip-date", True)
  mol = indigo.loadMolecule("c1ccccc1.CCC.O.N.P")
  mol.layout()
  saver.append(mol)
  sgroup1 = mol.addDataSGroup([6, 7, 8], [6, 7], "SG", "a")
  sgroup2 = mol.addDataSGroup([9], [], "ID", "b")
  sgroup3 = mol.addDataSGroup([10], [], "ID", "c")
  sgroup4 = mol.addDataSGroup([11], [], "ID", "d")
  print mol.molfile()
  saver.append(mol)
  sgroup2.setDataSGroupXY(13, 1)
  sgroup3.setDataSGroupXY(.3, .3, "relative")
  sgroup4.setDataSGroupXY(5, 6, "absolute")
  print mol.molfile()
  saver.append(mol)
testSGroupsInstrumentation()
