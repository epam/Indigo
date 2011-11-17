import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
   
indigo.setOption("render-output-format", "png")
def testRenderRGroups (filename, idx):
  mol = indigo.loadMoleculeFromFile(filename)
  qmol = indigo.loadQueryMoleculeFromFile(filename)
  renderer.renderToFile(mol, joinPath("out/rgroup-mol-{0}.png".format(idx)))
  renderer.renderToFile(qmol, joinPath("out/rgroup-qmol-{0}.png".format(idx)))
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  renderer.renderToFile(mol, joinPath("out/noap-mol-{0}.png".format(idx)))
  renderer.renderToFile(qmol, joinPath("out/noap-qmol-{0}png".format(idx)))
  
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  renderer.renderToFile(mol, joinPath("out/norgroup-mol-{0}.png".format(idx)))
  renderer.renderToFile(qmol, joinPath("out/norgroup-qmol-{0}.png".format(idx)))
  print(idx + " OK")
testRenderRGroups(joinPath("molecules/recursive1.mol"), "rec1")
testRenderRGroups(joinPath("molecules/r_occur.mol"), "occur")
testRenderRGroups(joinPath("molecules/r_resth.mol"), "resth")
testRenderRGroups(joinPath("molecules/r1-2ap-aal.mol"), "2ap")
