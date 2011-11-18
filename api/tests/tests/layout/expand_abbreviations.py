import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "true")
  
mol_file_name = joinPath("out/abbreviations_super_mol.mol")
  
m_sets = [
   joinPath('molecules/abbreviations_test.mol'),
   joinPath('molecules/abbreviations_tests.sdf'),
   mol_file_name
]
if not os.path.exists("out"):
   os.makedirs("out")
   
saver = indigo.createFileSaver(joinPath("out/abbreviations_test_out.sdf"), "SDF")

abbr_dict = { 
   ( "NO2", "O2N" ) : "[O-][N+](*)=O", 
   ( "Ph", "Ph" ) : "*C1=CC=CC=C1",
   ( "COOH", "HOOC" ) : "OC(*)=O",
   ( "Si-Pr", "i-PrS" ) : "CC(C)S*",
   ( "SiPr", "iPrS" ) : "CC(C)S*",
   ( "COOMe", "MeOOC" ) : "COC(*)=O",
   ( "COOEt", "EtOOC" ) : "CCOC(*)=O",
   ( "CN", "NC" ) : "*C#N", 
#   ( "NC", "CN" ) : "*[N+]#[C-]", 
   ( "CF3", "F3C" ) : "FC(F)(F)*", 
   ( "SO3H", "HO3S" ) : "OS(*)(=O)=O",
   ( "SO2H", "HO2S" ) : "OS(*)=O",
   ( "NCO", "OCN" ) : "*N=C=O",
   ( "ONO", "ONO" ) : "*ON=O",
   ( "ONO2", "O2NO" ) : "*ON(=O)=O",
   ( "PO3H2", "H2O3P" ) : "OP(O)(*)=O",
   ( "OTf", "TfO" ) : "FC(F)(F)S(=O)(=O)O*",
   ( "OBz", "BzO" ) : "*OC(=O)C1=CC=CC=C1",
   ( "OiPr", "iPrO") : "CC(C)O*",
   ( "OTMS", "TMSO" ) : "C[Si](C)(C)O*",
}

left_abbr_dict = dict()
right_abbr_dict = dict()
all_abbr_dict = dict()
for k in abbr_dict.keys():
    left, right = k
    left_abbr_dict[left] = abbr_dict[k]
    all_abbr_dict[left] = abbr_dict[k]
    right_abbr_dict[right] = abbr_dict[k]
    all_abbr_dict[right] = abbr_dict[k]
    
#for k in all_abbr_dict:
#    print("         abbr_map[\"%s\"] = \"%s\";" % (k, all_abbr_dict[k]))
    
# Create molecule for the abbreviation testing
m = indigo.createMolecule()
natoms = len(abbr_dict)
atoms = []
for i in range(natoms):
    a = m.addAtom("C")
    atoms.append(a.index())
    x = i % 5
    y = i / 5
    a.setXYZ(x * 7, y * 6, 0)
        
for i in range(0, natoms):
    left, right = abbr_dict.keys()[i]
    a1 = m.getAtom(atoms[i])
    x, y, z = a1.xyz()
    a2 = m.addAtom(left)
    a2.setXYZ(x + 2, y, z)
    
    a1.addBond(a2, 1)
    a3 = m.addAtom(right)
    a3.setXYZ(x - 2, y, z)
    a1.addBond(a3, 1)
    
m.layout()
f = file(mol_file_name, "w")
f.write(m.molfile())
f.close()

def expand_abbreviations(m):
   to_layout = []
   to_replace = [a for a in m.iterateAtoms() if a.symbol() in all_abbr_dict]
   print([a.symbol() for a in to_replace])
   for a in to_replace:
      name = a.symbol()
      expanded = indigo.loadMolecule(all_abbr_dict[a.symbol()])
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
   print("*** Molecule set %s ***" % relativePath(mset))
   idx = 1
   for m in indigo.iterateSDFile(mset):
      try:
         expand_abbreviations(m)   
         print("Smiles: %s" % m.smiles())
      except IndigoException, e:
         print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
      idx += 1
