import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists("out"):
   os.makedirs("out")
   
indigo.setOption("render-output-format", "png")
def testRenderRGroups (filename, idx):
  mol = indigo.loadMoleculeFromFile(filename)
  qmol = indigo.loadQueryMoleculeFromFile(filename)
  renderer.renderToFile(mol, "out/rgroup-mol-" + idx + ".png")
  renderer.renderToFile(qmol, "out/rgroup-qmol-" + idx + ".png")
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  renderer.renderToFile(mol, "out/noap-mol-" + idx + ".png")
  renderer.renderToFile(qmol, "out/noap-qmol-" + idx + ".png")
  
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  renderer.renderToFile(mol, "out/norgroup-mol-" + idx + ".png")
  renderer.renderToFile(qmol, "out/norgroup-qmol-" + idx + ".png")
  print idx + " OK"
testRenderRGroups("molecules/recursive1.mol", "rec1")
testRenderRGroups("molecules/r_occur.mol", "occur")
testRenderRGroups("molecules/r_resth.mol", "resth")
testRenderRGroups("molecules/r1-2ap-aal.mol", "2ap")
