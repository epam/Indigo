import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out/rgroups/")):
   os.makedirs(joinPath("out/rgroups/"))
   
def testRenderRGroups(filename, idx, format):
  indigo.setOption("render-output-format", format)
  mol = indigo.loadMoleculeFromFile(filename)
  qmol = indigo.loadQueryMoleculeFromFile(filename)
  renderer.renderToFile(mol, joinPath("out/rgroups/rgroup-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/rgroups/rgroup-qmol-%s.%s" % (idx, format)))
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.clearAttachmentPoints()
  renderer.renderToFile(mol, joinPath("out/rgroups/noap-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/rgroups/noap-qmol-%s.%s" % (idx, format)))
  
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  for rgp in qmol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      frag.remove()
  renderer.renderToFile(mol, joinPath("out/rgroups/norgroup-mol-%s.%s" % (idx, format)))
  renderer.renderToFile(qmol, joinPath("out/rgroups/norgroup-qmol-%s.%s" % (idx, format)))
  print(idx + " OK")
  
testRenderRGroups(joinPath("molecules/recursive1.mol"), "rec1", "png")
testRenderRGroups(joinPath("molecules/r_occur.mol"), "occur", "png")
testRenderRGroups(joinPath("molecules/r_resth.mol"), "resth", "png")
testRenderRGroups(joinPath("molecules/r1-2ap-aal.mol"), "2ap", "png")
testRenderRGroups(joinPath("molecules/recursive1.mol"), "rec1", "svg")
testRenderRGroups(joinPath("molecules/r_occur.mol"), "occur", "svg")
testRenderRGroups(joinPath("molecules/r_resth.mol"), "resth", "svg")
testRenderRGroups(joinPath("molecules/r1-2ap-aal.mol"), "2ap", "svg")
for outputFile in os.listdir(joinPath('out/rgroups/')):
    if os.path.splitext(outputFile)[1] == '.svg':
        print('\nout/rgroups/%s:\n' % outputFile)
        with open(joinPath('out/%s' % outputFile)) as f:
            print(f.read())