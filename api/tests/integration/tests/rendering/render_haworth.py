import sys, os
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
indigo.setOption("stereochemistry-bidirectional-mode", True)
indigo.setOption("stereochemistry-detect-haworth-projection", True)

renderer = IndigoRenderer(indigo)
indigo.setOption("render-coloring", True)
indigo.setOption("render-output-format", "png")
indigo.setOption("render-image-size", 500, 500)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("render-relative-thickness", 1.0)

m = indigo.loadMolecule("""


 16 16  0  0  0  0  0  0  0  0999 V2000
   -2.3212    0.6468    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.1026    0.2566    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4962    0.6468    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9442    0.0337    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5507   -0.4330    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4962   -0.5793    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.3212    1.4718    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.4074   -0.1736    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9442    0.8587    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   -1.0304   -0.7867    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4962   -1.4043    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7511    0.2053    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5507   -1.2580    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   -2.9494    1.0548    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -3.0357   -0.5906    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5507    0.3919    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
  2  1  1  0  0  0  0
  2  5  1  1  0  0  0
  4  6  1  1  0  0  0
  5  6  1  0  0  0  0
  1  3  1  0  0  0  0
  3  4  1  0  0  0  0
  1  7  1  0  0  0  0
  2 15  1  0  0  0  0
  5 13  1  0  0  0  0
  6 11  1  0  0  0  0
  4  9  1  0  0  0  0
  1  8  1  0  0  0  0
  4 10  1  0  0  0  0
  6 12  1  0  0  0  0
  5 16  1  0  0  0  0
  2 14  1  0  0  0  0
M  END
""")

renderer.renderToFile(m, joinPath('out/render_haworth.png'))
print(checkImageSimilarity('render_haworth.png'))
if isIronPython():
    renderer.Dispose()