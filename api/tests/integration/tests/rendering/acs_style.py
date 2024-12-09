import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoRenderer, isIronPython, joinPathPy  # noqa
from rendering import checkImageSimilarity

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

mol_data = """
  Ketcher  9262423222D 1   1.00000     0.00000     0

 12 11  0  0  0  0  0  0  0  0999 V2000
   21.7000  -12.0410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   22.2000  -11.1750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.2000  -11.1750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.7000  -10.3090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   23.7000  -12.0410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   21.7000  -10.3090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.7000  -10.3090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   24.7000  -10.3090    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.7000  -12.0410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   24.7000  -12.0410    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   20.2000   -9.4429    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   25.2000   -9.4429    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  2  1  1  0     0  0
  2  3  2  0     0  0
  3  4  1  0     0  0
  3  5  1  0     0  0
  2  6  1  0     0  0
  6  7  1  1     0  0
  4  8  1  6     0  0
  1  9  1  0     0  0
  5 10  1  0     0  0
  7 11  1  0     0  0
  8 12  1  0     0  0
M  END

"""

indigo = Indigo()
renderer = IndigoRenderer(indigo)

print("****** Default rendering settings *****")

indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-output-format", "png")
mol = indigo.loadMolecule(mol_data)
renderer.renderToFile(mol, joinPathPy("out/acs_style_default.png", __file__))
print(checkImageSimilarity("acs_style_default.png"))

print("****** Changed ACS settings *****")
indigo.setOption("bond-length", "1.2")
indigo.setOption("bond-length-unit", "inch")
indigo.setOption("render-bond-spacing", "0.5")
indigo.setOption("render-hash-spacing", "10")
indigo.setOption("render-stereo-bond-width", "30")
indigo.setOption("render-font-size", "20")
indigo.setOption("render-font-size-sub", "30")
renderer.renderToFile(mol, joinPathPy("out/acs_style_changed.png", __file__))
print(checkImageSimilarity("acs_style_changed.png"))

print("****** Issue 2447 wrong stereobond width *****")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
indigo.setOption("ignore-stereochemistry-errors", "true")
indigo.setOption("bond-length-unit", "px")
indigo.setOption("bond-length", "40")
indigo.setOption("render-bond-thickness", "2")
indigo.setOption("render-bond-thickness-unit", "px")
indigo.setOption("render-stereo-bond-width", "6")
indigo.setOption("render-stereo-bond-width-unit", "px")
indigo.setOption("render-hash-spacing", "1.2")
indigo.setOption("render-hash-spacing-unit", "px")
name = "issue_2447"
fname = name + ".ket"
mol = indigo.loadMoleculeFromFile(joinPathPy("molecules/" + fname, __file__))
pngname = name + ".png"
renderer.renderToFile(mol, joinPathPy("out/" + pngname, __file__))
print(checkImageSimilarity(pngname))


if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
