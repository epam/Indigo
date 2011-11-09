import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
   
def testAlignAtoms ():
  query = indigo.loadSmarts("[#7]1~[#6]~[#6]~[#7]~[#6]~[#6]2~[#6]~[#6]~[#6]~[#6]~[#6]~1~2")
  sdfout = indigo.writeFile(joinPath("out/aligned.sdf"))
  xyz = []
  collection = indigo.createArray()
  refatoms = []
  for structure in indigo.iterateSDFile(joinPath("molecules/benzodiazepine.sdf.gz")):
    match = indigo.substructureMatcher(structure).match(query)
    if not match:
      print "structure not matched, this is unexpected"
      return
    if not structure.index():
      for atom in query.iterateAtoms():
        xyz.extend(match.mapAtom(atom).xyz())
    else:
      atoms = [match.mapAtom(atom).index() for atom in query.iterateAtoms()]
      x = structure.alignAtoms(atoms, xyz)
      print '%.6f' % x
    
      structure.foldHydrogens()
      sdfout.sdfAppend(structure)
    refatoms.append(match.mapAtom(query.getAtom(0)).index())
    collection.arrayAdd(structure)
    if structure.index() == 15:
      break
  
  indigo.setOption("render-output-format", "png")
  indigo.setOption("render-highlight-thickness-enabled", "true")
  indigo.setOption("render-image-size", "400, 400")
  indigo.setOption("render-grid-title-property", "PUBCHEM_COMPOUND_CID")
  indigo.setOption("render-grid-title-font-size", "10")
  indigo.setOption("render-grid-title-offset", "2")
  indigo.setOption("render-grid-title-alignment", 0.5)
  indigo.setOption("render-coloring", "true")
  renderer.renderGridToFile(collection, None, 4, joinPath("out/grid.png"))
  renderer.renderGridToFile(collection, refatoms, 4, joinPath("out/grid1.png"))
testAlignAtoms()
