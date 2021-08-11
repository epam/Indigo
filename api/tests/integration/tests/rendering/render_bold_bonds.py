import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = joinPath("out")

if not os.path.exists(out_dir):
    try:
        os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("ignore-stereochemistry-errors", True)

m = indigo.loadMoleculeFromFile(joinPath("molecules", "bold-bonds.mol"))

for v in [ "unset", "true", "false" ]:
    if v != "unset":
        indigo.setOption("render-bold-bond-detection", v)

    renderer.renderToFile(m, joinPath("out/bold-bonds-%s.png" % v))
    print(checkImageSimilarity('bold-bonds-%s.png' % v))
renderer.Dispose()