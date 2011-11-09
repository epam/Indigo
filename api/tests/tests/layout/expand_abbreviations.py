import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "true")
  
m_sets = [
   joinPath('molecules/abbreviations_test.mol'),
   joinPath('molecules/abbreviations_tests.sdf')
]
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
   
saver = indigo.createFileSaver(joinPath("out/abbreviations_test_out.sdf"), "SDF")
abbr_dict = { 
   "NO2" : "[O-][N+]([*])=O", 
   "Ph" : "*C1=CC=CC=C1",
   "COOH" : "OC(*)=O",
   "SiPr" : "CC(C)S*",
}
def expand_abbreviations(m):
   to_layout = []
   to_replace = [a for a in m.iterateAtoms() if a.symbol() in abbr_dict]
   print [a.symbol() for a in to_replace]
   for a in to_replace:
      name = a.symbol()
      expanded = indigo.loadMolecule(abbr_dict[a.symbol()])
      attachment = None
      for aa in expanded.iterateAtoms():
         if aa.isRSite():
            for nei in aa.iterateNeighbors():
               attachment = nei
            aa.remove()
            break
      
      atom_from = None
      order = None
      for nei in a.iterateNeighbors():
         atom_from = nei
         order = nei.bond().bondOrder()
      pos = a.xyz()
      a.remove()
      mapping = m.merge(expanded)
      mapped = mapping.mapAtom(attachment)
      atom_from.addBond(mapped, order)
      mapped.setXYZ(pos[0], pos[1], pos[2])
      superatom_vertices = []
      for aa in expanded.iterateAtoms():
         midx = mapping.mapAtom(aa).index()
         superatom_vertices.append(midx)
         if aa.index() != attachment.index():
            to_layout.append(midx)
      m.addSuperatom(superatom_vertices, name)
   subm = m.getSubmolecule(to_layout)
   subm.layout()
   
   saver.append(m)
for mset in m_sets:
   print "*** Molecule set %s ***" % relativePath(mset)
   idx = 1
   for m in indigo.iterateSDFile(mset):
      try:
         expand_abbreviations(m)   
         print("Smiles: %s" % m.smiles())
      except IndigoException, e:
         print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
      idx += 1
