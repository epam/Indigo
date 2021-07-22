import os
import sys
import itertools
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("render-label-mode", "all")
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "1, 1, 1")

m = indigo.loadMoleculeFromFile(joinPath("molecules", "atom-coloring.mol"))   
renderer.renderToFile(m, joinPath("out/atom-coloring-no.png"))
print(checkImageSimilarity('atom-coloring-no.png'))

m.addDataSGroup([1, 2, 3], [], "color", "0.155, 0.55, 0.955")
indigo.setOption("render-atom-color-property", "color")

renderer.renderToFile(m, joinPath("out/atom-coloring-1.png"))
print(checkImageSimilarity('atom-coloring-1.png'))

m.addDataSGroup([4, 5, 6], [], "color", "0.955, 0.155, 0.155")

renderer.renderToFile(m, joinPath("out/atom-coloring-2.png"))
print(checkImageSimilarity('atom-coloring-2.png'))

print("*** Reaction atom coloring ***")
rxn = indigo.createReaction()
rxn.addReactant(m)
rxn.addProduct(m)
rxn.addCatalyst(m)
renderer.renderToFile(rxn, joinPath("out/rxn-atom-coloring.png"))
print(checkImageSimilarity('rxn-atom-coloring.png'))
