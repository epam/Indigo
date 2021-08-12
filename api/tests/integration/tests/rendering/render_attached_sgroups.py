import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

def testRenderAttachedSGroups(smiles):
    mol = indigo.loadMolecule(smiles)
    mol.layout()
    for atom in mol.iterateAtoms():
        mol.addDataSGroup([atom.index()], [], "some", str(atom.index() + 1))
    indigo.setOption("render-data-sgroup-color", 0.8, 0.2, 0.8)
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(mol, joinPathPy("out/mol-with-indices.png", __file__))
    print(checkImageSimilarity('mol-with-indices.png'))
    indigo.setOption("render-output-format", "svg")
    renderer.renderToFile(mol, joinPathPy("out/mol-with-indices.svg", __file__))
    print(checkImageSimilarity('mol-with-indices.svg'))

testRenderAttachedSGroups("N1C=CC=CC1C{-}c1ncccc{+n}1")
#testRenderAttachedSGroups("C{-}c1ccccc{+n}1")
print("Done")