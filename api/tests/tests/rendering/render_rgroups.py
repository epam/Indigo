import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))

def testRenderRGroups(filename, idx, format):
  indigo.setOption("render-output-format", format)
  mol = indigo.loadMoleculeFromFile(filename)
  qmol = indigo.loadQueryMoleculeFromFile(filename)
  renderer.renderToFile(mol, joinPath("out/rgroup-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/rgroup-qmol-%s.%s" % (idx, format)))
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  renderer.renderToFile(mol, joinPath("out/noap-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/noap-qmol-%s.%s" % (idx, format)))
  
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  renderer.renderToFile(mol, joinPath("out/norgroup-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/norgroup-qmol-%s.%s" % (idx, format)))
  print(idx + " OK")
  
testRenderRGroups(joinPath("molecules/recursive1.mol"), "rec1", "png")
testRenderRGroups(joinPath("molecules/r_occur.mol"), "occur", "png")
testRenderRGroups(joinPath("molecules/r_resth.mol"), "resth", "png")
testRenderRGroups(joinPath("molecules/r1-2ap-aal.mol"), "2ap", "png")

testRenderRGroups(joinPath("molecules/recursive1.mol"), "rec1", "svg")
testRenderRGroups(joinPath("molecules/r_occur.mol"), "occur", "svg")
testRenderRGroups(joinPath("molecules/r_resth.mol"), "resth", "svg")
testRenderRGroups(joinPath("molecules/r1-2ap-aal.mol"), "2ap", "svg")

for outputFile in os.listdir(joinPath('out')):
    if os.path.splitext(outputFile) == '.svg':
        print('\nout/%s:\n' % outputFile)
        with open(joinPath('out/%s' % outputFile)) as f:
            print(f.read())
    else:
        print(outputFile)