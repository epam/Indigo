import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
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
# renderer.renderToFile(mol, joinPath("out/image-no-size.png"))
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
renderer.renderToFile(mol, joinPath("out/rsite_highlighted.svg"))
print(checkImageSimilarity('rsite_highlighted.svg'))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPath('out/rsite_highlighted.png'))
print(checkImageSimilarity('rsite_highlighted.png'))

indigo.setOption("ignore-stereochemistry-errors", "true")
mol = indigo.loadQueryMoleculeFromFile(joinPath("molecules/bonds.mol"))
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPath("out/bonds.svg"))
print(checkImageSimilarity('bonds.svg'))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPath("out/bonds.png"))
print(checkImageSimilarity('bonds.png'))

m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
indigo.setOption("render-output-format", "png")
renderer.renderToFile(m, joinPath("out/ind-514-output.png"))
print(checkImageSimilarity('ind-514-output.png'))
if isIronPython():
    renderer.Dispose()

indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-background-color", "255, 255, 255")
m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
renderer.renderToFile(m, joinPath("out/test-size-01.png"))
print(checkImageSimilarity('test-size-01.png'))
indigo.setOption("render-image-width", "300")
renderer.renderToFile(m, joinPath("out/test-size-02.png"))
print(checkImageSimilarity('test-size-02.png'))
indigo.setOption("render-image-max-height", "50")
renderer.renderToFile(m, joinPath("out/test-size-03.png"))
print(checkImageSimilarity('test-size-03.png'))

indigo.setOption("render-background-color", "255, 255, 255")
m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
indigo.setOption("render-image-width", "427")
indigo.setOption("render-image-height", "300")
renderer.renderToFile(m, joinPath("out/test-size-04.png"))
print(checkImageSimilarity('test-size-04.png'))
indigo.setOption("render-image-max-width", "50")
indigo.setOption("render-image-max-height", "300")
renderer.renderToFile(m, joinPath("out/test-size-05.png"))
print(checkImageSimilarity('test-size-05.png'))

print("*** render-bond-length with render-image-size ***")
indigo.setOption("render-output-format", "png")
indigo.setOption("render-background-color", "1, 1, 1")
indigo.setOption("render-relative-thickness", "2.0")
indigo.setOption("render-bond-length", "30.0")
indigo.setOption("render-image-max-width", "1280")
indigo.setOption("render-image-max-height", "1024")
indigo.setOption("render-image-size", 1280, 1024)


mol = indigo.loadReaction("C1C(CCN)CCC1>>C1CCCCCC1")
renderer.renderToFile(mol, joinPath("out/image-size-bond-length.png"))
print(checkImageSimilarity('image-size-bond-length.png'))

mol = indigo.loadReaction("C>>N")
renderer.renderToFile(mol, joinPath("out/image-size-bond-length-2.png"))
print(checkImageSimilarity('image-size-bond-length-2.png'))
if isIronPython():
    renderer.Dispose()
print("****** Bug with options mismatch *****")
indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-background-color", "255, 255, 255")

m = indigo.loadMolecule("CCn1c2nc[nH]c2c(=O)[nH]c1=O")
options = [ 30.0, 30, "30", "30.0" ]
for idx, opt in enumerate(options):
    indigo.setOption("render-bond-length", opt)
    renderer.renderToFile(m, joinPath("out/bond-length-options-%d.png" % idx))
    print(checkImageSimilarity('bond-length-options-%d.png' % idx, 'bond-length-options-0.png'))
if isIronPython():
    renderer.Dispose()
print("****** Reference count *****")
indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "png")
m = indigo.loadMolecule("CCCC")
# Make sure that there are no leak of IndigoObjects
cnt0 = indigo.countReferences()
buf = renderer.renderToBuffer(m)
cnt1 = indigo.countReferences()
# Potentialy GC can happen here, but it should not increase the number of Indigo objects
# Usually these counters should be equal
assert cnt0 >= cnt1
if isIronPython():
    renderer.Dispose()

print("****** Smart layout*****")
indigo = Indigo()
renderer = IndigoRenderer(indigo)
mol = indigo.loadMolecule("C1OCCOCCOCCOCCOCCOCCOCCOCCOCCOC1")
indigo.setOption("render-output-format", "png")
indigo.setOption("smart-layout", "true")
renderer.renderToFile(mol, joinPath("out/smart-layout-crown.png"))
print(checkImageSimilarity('smart-layout-crown.png'))
if isIronPython():
    renderer.Dispose()