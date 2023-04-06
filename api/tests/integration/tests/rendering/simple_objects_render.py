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

# print("****** Reset options check *****")
# indigo = Indigo()
# renderer = IndigoRenderer(indigo)
# indigo.setOption("render-output-format", "png")
# indigo.setOption("render-image-size", 1280, 1024)
# mol = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
# indigo.resetOptions()
# renderer.renderToFile(mol, joinPathPy("out/image-no-size.png", __file__))
# TODO: Use PIL only on Python and something else in Java or C#
# from PIL import Image
# im=Image.open("out/image-no-size.png")
# if im.size != (1280, 1024):
# 	print("Reset renderer optiont test OK")
# else:
# 	print("Reset renderer optiont test FAIL")


indigo = Indigo()
renderer = IndigoRenderer(indigo)

mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/simple.ket", __file__))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPathPy("out/simple.png", __file__))
print(checkImageSimilarity("simple.png"))

rea = indigo.loadReactionFromFile(joinPathPy("reactions/multi.ket", __file__))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(rea, joinPathPy("out/multi.png", __file__))
print(checkImageSimilarity("multi.png"))

rea = indigo.loadReactionFromFile(
    joinPathPy("reactions/single_arrow.ker", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(rea, joinPathPy("out/single_arrow.png", __file__))
print(checkImageSimilarity("single_arrow.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
