import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists("out"):
   os.makedirs("out")
def testRenderAttachedSGroups (smiles):
  mol = indigo.loadMolecule(smiles)
  #mol.layout()
  for atom in mol.iterateAtoms():
    mol.addDataSGroup([atom.index()], [], "some", str(atom.index() + 1))
  indigo.setOption("render-output-format", "png")
  indigo.setOption("render-data-sgroup-color", 0.8, 0.2, 0.8)
  renderer.renderToFile(mol, "out/mol-with-indices.png")
testRenderAttachedSGroups("N1C=CC=CC1C{-}c1ncccc{+n}1")
#testRenderAttachedSGroups("C{-}c1ccccc{+n}1")
print "Done"
