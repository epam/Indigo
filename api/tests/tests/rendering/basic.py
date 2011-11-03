import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = "out"
if not os.path.exists(out_dir):
   os.makedirs(out_dir)
mol = indigo.loadMolecule("CCNNCN")
a1 = mol.getAtom(1)
a1.setRSite("R4")
a1.highlight()
a2 = mol.getAtom(3)
a2.setRSite("R3")
a2.highlight()
print(mol.smiles())
indigo.setOption("render-output-format", "png")
indigo.setOption("render-background-color", "255, 255, 255")
renderer.renderToFile(mol, "%s/rsite_highlighted.png" % out_dir)
