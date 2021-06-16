import os
import sys
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

def renderRxnfile(filename, outfile):
    rxn = indigo.loadReactionFromFile(filename)
    indigo.setOption("render-label-mode", "hetero")
    indigo.setOption("render-bond-length", "40.0")
    indigo.setOption("render-output-format", "svg")
    renderer.renderToFile(rxn, joinPath("out/%s.svg" % (outfile)))
    print(checkImageSimilarity('%s.svg' % outfile))
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(rxn, joinPath("out/%s.png" % (outfile)))
    print(checkImageSimilarity('%s.png' % outfile))

    rxn2 = indigo.loadReaction(rxn.smiles())
    indigo.setOption("render-output-format", "svg")
    renderer.renderToFile(rxn2, joinPath("out/%s-smiles.svg" % (outfile)))
    print(checkImageSimilarity('%s.svg' % outfile))
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(rxn2, joinPath("out/%s-smiles.png" % (outfile)))
    print(checkImageSimilarity('%s.png' % outfile))


    rxn3 = indigo.loadReactionFromFile(filename)
    indigo.setOption("render-output-format", "cdxml")
    indigo.setOption("render-bond-length", 30)
    renderer.renderToFile(rxn3, joinPath("out/%s.cdxml" % (outfile)))


indigo.setOption("render-output-format", "png")
for size in range(10, 100, 5):
    indigo.setOption("render-bond-length", "%f" % (size))
    m = indigo.loadReactionFromFile(joinPath("reactions/adama_reaction.rxn"))
    renderer.renderToFile(m, joinPath("out/adama_reaction-size-%03d.png" % (size)))

renderRxnfile(joinPath("reactions/adama_reaction.rxn"), "adama_reaction")
renderRxnfile(joinPath("reactions/epoxy.rxn"), "epoxy")



print("Done")