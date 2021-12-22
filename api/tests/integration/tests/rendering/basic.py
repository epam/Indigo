import errno
import os
import sys

sys.path.append("../../common")
from env_indigo import *
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

mol = indigo.loadMolecule("CCNNCN")
a1 = mol.getAtom(1)
a1.setRSite("R4")
a1.highlight()
a2 = mol.getAtom(3)
a2.setRSite("R3")
a2.highlight()
print(mol.smiles())

indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPathPy("out/rsite_highlighted.svg", __file__))
print(checkImageSimilarity("rsite_highlighted.svg"))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPathPy("out/rsite_highlighted.png", __file__))
print(checkImageSimilarity("rsite_highlighted.png"))

indigo.setOption("ignore-stereochemistry-errors", "true")
mol = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/bonds.mol", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPathPy("out/bonds.svg", __file__))
print(checkImageSimilarity("bonds.svg"))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPathPy("out/bonds.png", __file__))
print(checkImageSimilarity("bonds.png"))

m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
indigo.setOption("render-output-format", "png")
renderer.renderToFile(m, joinPathPy("out/ind-514-output.png", __file__))
print(checkImageSimilarity("ind-514-output.png"))

indigo.resetOptions()

indigo.setOption("render-background-color", "255, 255, 255")
m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
renderer.renderToFile(m, joinPathPy("out/test-size-01.png", __file__))
print(checkImageSimilarity("test-size-01.png"))
indigo.setOption("render-image-width", "300")
renderer.renderToFile(m, joinPathPy("out/test-size-02.png", __file__))
print(checkImageSimilarity("test-size-02.png"))
indigo.setOption("render-image-max-height", "50")
renderer.renderToFile(m, joinPathPy("out/test-size-03.png", __file__))
print(checkImageSimilarity("test-size-03.png"))

indigo.setOption("render-background-color", "255, 255, 255")
m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
indigo.setOption("render-image-width", "427")
indigo.setOption("render-image-height", "300")
renderer.renderToFile(m, joinPathPy("out/test-size-04.png", __file__))
print(checkImageSimilarity("test-size-04.png"))
indigo.setOption("render-image-max-width", "50")
indigo.setOption("render-image-max-height", "300")
renderer.renderToFile(m, joinPathPy("out/test-size-05.png", __file__))
print(checkImageSimilarity("test-size-05.png"))

print("*** render-bond-length with render-image-size ***")
indigo.setOption("render-output-format", "png")
indigo.setOption("render-background-color", "1, 1, 1")
indigo.setOption("render-relative-thickness", "2.0")
indigo.setOption("render-bond-length", "30.0")
indigo.setOption("render-image-max-width", "1280")
indigo.setOption("render-image-max-height", "1024")
indigo.setOption("render-image-size", 1280, 1024)


mol = indigo.loadReaction("C1C(CCN)CCC1>>C1CCCCCC1")
renderer.renderToFile(
    mol, joinPathPy("out/image-size-bond-length.png", __file__)
)
print(checkImageSimilarity("image-size-bond-length.png"))

mol = indigo.loadReaction("C>>N")
renderer.renderToFile(
    mol, joinPathPy("out/image-size-bond-length-2.png", __file__)
)
print(checkImageSimilarity("image-size-bond-length-2.png"))

print("****** Bug with options mismatch *****")
indigo.resetOptions()
indigo.setOption("render-background-color", "255, 255, 255")

m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
options = [30.0, 30, "30", "30.0"]
for idx, opt in enumerate(options):
    indigo.setOption("render-bond-length", opt)
    renderer.renderToFile(
        m, joinPathPy("out/bond-length-options-%d.png" % idx, __file__)
    )
    print(
        checkImageSimilarity(
            "bond-length-options-%d.png" % idx, "bond-length-options-0.png"
        )
    )

print("****** Reference count *****")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
m = indigo.loadMolecule("CCCC")
# Make sure that there are no leak of IndigoObjects
cnt0 = indigo.countReferences()
buf = renderer.renderToBuffer(m)
cnt1 = indigo.countReferences()
# Potentialy GC can happen here, but it should not increase the number of Indigo objects
# Usually these counters should be equal
assert cnt0 >= cnt1

print("****** Smart layout*****")
indigo.resetOptions()
mol = indigo.loadMolecule("C1OCCOCCOCCOCCOCCOCCOCCOCCOCCOC1")
indigo.setOption("render-output-format", "png")
indigo.setOption("smart-layout", "true")
renderer.renderToFile(mol, joinPathPy("out/smart-layout-crown.png", __file__))
print(checkImageSimilarity("smart-layout-crown.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
