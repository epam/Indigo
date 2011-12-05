import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath('out')):
   os.makedirs(joinPath('out'))
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
renderer.renderToFile(mol, joinPath("out/rsite_highlighted.png"))
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPath("out/rsite_highlighted.svg"))
print('\nout/rsite_highlighted.svg:\n')
with open(joinPath('out/rsite_highlighted.svg')) as f:
    print(f.read())
    
indigo.setOption("render-output-format", "png")
indigo.setOption("ignore-stereochemistry-errors", "true")
mol = indigo.loadQueryMoleculeFromFile(joinPath("molecules/bonds.mol"))
renderer.renderToFile(mol, joinPath("out/bonds.png"))
