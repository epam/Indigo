import errno
import itertools
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("render-label-mode", "all")
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "1, 1, 1")

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/atom-coloring.mol", __file__)
)
renderer.renderToFile(m, joinPathPy("out/atom-coloring-no.png", __file__))
print(checkImageSimilarity("atom-coloring-no.png"))

m.addDataSGroup([1, 2, 3], [], "color", "0.155, 0.55, 0.955")
indigo.setOption("render-atom-color-property", "color")

renderer.renderToFile(m, joinPathPy("out/atom-coloring-1.png", __file__))
print(checkImageSimilarity("atom-coloring-1.png"))

m.addDataSGroup([4, 5, 6], [], "color", "0.955, 0.155, 0.155")

renderer.renderToFile(m, joinPathPy("out/atom-coloring-2.png", __file__))
print(checkImageSimilarity("atom-coloring-2.png"))

print("*** Reaction atom coloring ***")
rxn = indigo.createReaction()
rxn.addReactant(m)
rxn.addProduct(m)
rxn.addCatalyst(m)
renderer.renderToFile(rxn, joinPathPy("out/rxn-atom-coloring.png", __file__))
print(checkImageSimilarity("rxn-atom-coloring.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
