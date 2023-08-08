import errno
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

out_dir = joinPathPy("out", __file__)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("treat-x-as-pseudoatom", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("ignore-stereochemistry-errors", True)

filename = "halide.mol"
file = joinPathPy("molecules/" + filename, __file__)
mol = indigo.loadQueryMoleculeFromFile(file)

indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol, joinPathPy(out_dir + "/" + filename + "_q.svg", __file__)
)
print(checkImageSimilarity("%s" % (filename + "_q.svg")))

indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol, joinPathPy(out_dir + "/" + filename + "_q.png", __file__)
)
print(checkImageSimilarity("%s" % (filename + "_q.png")))

print("   OK")

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
