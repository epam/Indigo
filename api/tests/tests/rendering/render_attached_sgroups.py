import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
   
def testRenderAttachedSGroups (smiles):
  mol = indigo.loadMolecule(smiles)
  #mol.layout()
  for atom in mol.iterateAtoms():
    mol.addDataSGroup([atom.index()], [], "some", str(atom.index() + 1))
  indigo.setOption("render-output-format", "png")
  indigo.setOption("render-data-sgroup-color", 0.8, 0.2, 0.8)
  renderer.renderToFile(mol, joinPath("out/mol-with-indices.png"))
  indigo.setOption("render-output-format", "svg")
  renderer.renderToFile(mol, joinPath("out/mol-with-indices.svg"))
  print('\nout/mol-with-indices.svg:\n')
  with open(joinPath('out/mol-with-indices.svg')) as f:
    print(f.read())
  
testRenderAttachedSGroups("N1C=CC=CC1C{-}c1ncccc{+n}1")
#testRenderAttachedSGroups("C{-}c1ccccc{+n}1")
print("Done")