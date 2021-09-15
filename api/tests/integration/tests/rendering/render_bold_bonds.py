import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = joinPathPy("out", __file__)

if not os.path.exists(out_dir):
    try:
        os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("ignore-stereochemistry-errors", True)

m = indigo.loadMoleculeFromFile(joinPathPy("molecules/bold-bonds.mol", __file__))

for v in [ "unset", "true", "false" ]:
    if v != "unset":
        indigo.setOption("render-bold-bond-detection", v)

    renderer.renderToFile(m, joinPathPy("out/bold-bonds-%s.png" % v, __file__))
    print(checkImageSimilarity('bold-bonds-%s.png' % v))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
