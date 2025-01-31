import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoRenderer, isIronPython, joinPathPy  # noqa
from rendering import checkImageSimilarity  # noqa

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

print("issue 2457 wrong chiral label position")
indigo.resetOptions()
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "png")
fname = "issue_2457"
png_fname = fname + ".png"
ket_fname = joinPathPy("reactions/%s.ket" % fname, __file__)
rxn = indigo.loadReactionFromFile(ket_fname)
renderer.renderToFile(rxn, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2444 wrong arrow and plus width")
indigo.resetOptions()
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "png")
indigo.setOption("render-bond-thickness", "10")
fname = "issue_2444"
png_fname = fname + ".png"
ket_fname = joinPathPy("reactions/%s.ket" % fname, __file__)
rxn = indigo.loadReactionFromFile(ket_fname)
renderer.renderToFile(rxn, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2512 two short equilibrium half arrows")
indigo.resetOptions()
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "png")
fname = "issue_2512"
png_fname = fname + ".png"
ket_fname = joinPathPy("reactions/%s.ket" % fname, __file__)
rxn = indigo.loadReactionFromFile(ket_fname)
renderer.renderToFile(rxn, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2513 elliptical-arc-arrow render error")
indigo.resetOptions()
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "png")
fname = "issue_2513"
png_fname = fname + ".png"
ket_fname = joinPathPy("reactions/%s.ket" % fname, __file__)
rxn = indigo.loadReactionFromFile(ket_fname)
renderer.renderToFile(rxn, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2741 missing stereochemistry")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
fname = "missing_stereochemistry_2741"
png_fname = fname + ".png"
cdxml_fname = joinPathPy("reactions/%s.cdxml" % fname, __file__)
rxn = indigo.loadReactionFromFile(cdxml_fname)
renderer.renderToFile(rxn, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
