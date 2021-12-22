import errno
import os
import sys

sys.path.append("../../common")
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


def renderRxnfile(filename, outfile):
    rxn = indigo.loadReactionFromFile(filename)
    indigo.setOption("render-label-mode", "hetero")
    indigo.setOption("render-bond-length", "40.0")
    indigo.setOption("render-output-format", "svg")
    renderer.renderToFile(rxn, joinPathPy("out/%s.svg" % (outfile), __file__))
    print(checkImageSimilarity("%s.svg" % outfile))
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(rxn, joinPathPy("out/%s.png" % (outfile), __file__))
    print(checkImageSimilarity("%s.png" % outfile))

    rxn2 = indigo.loadReaction(rxn.smiles())
    indigo.setOption("render-output-format", "svg")
    renderer.renderToFile(
        rxn2, joinPathPy("out/%s-smiles.svg" % (outfile), __file__)
    )
    print(checkImageSimilarity("%s.svg" % outfile))
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(
        rxn2, joinPathPy("out/%s-smiles.png" % (outfile), __file__)
    )
    print(checkImageSimilarity("%s.png" % outfile))

    rxn3 = indigo.loadReactionFromFile(filename)
    indigo.setOption("render-output-format", "cdxml")
    indigo.setOption("render-bond-length", 30)
    renderer.renderToFile(
        rxn3, joinPathPy("out/%s.cdxml" % (outfile), __file__)
    )


indigo.setOption("render-output-format", "png")
for size in range(10, 100, 5):
    indigo.setOption("render-bond-length", "%f" % (size))
    m = indigo.loadReactionFromFile(
        joinPathPy("reactions/adama_reaction.rxn", __file__)
    )
    renderer.renderToFile(
        m, joinPathPy("out/adama_reaction-size-%03d.png" % (size), __file__)
    )

renderRxnfile(
    joinPathPy("reactions/adama_reaction.rxn", __file__), "adama_reaction"
)
renderRxnfile(joinPathPy("reactions/epoxy.rxn", __file__), "epoxy")
print("Done")

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
