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

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
renderer = IndigoRenderer(indigo)

rea = indigo.loadReactionFromFile(
    joinPathPy("reactions/catalist_below.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(rea, joinPathPy("out/catalist_below.png", __file__))
print(checkImageSimilarity("catalist_below.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
