import sys, os
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

out_dir = joinPath("out")

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
    
indigo.setOption("treat-x-as-pseudoatom", "true")
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("ignore-stereochemistry-errors", True)

filename = "halide.mol"
file = joinPath("molecules", filename)
mol = indigo.loadQueryMoleculeFromFile(file)

indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPath(out_dir, filename + "_q.svg"))
print(checkImageSimilarity('%s' % (filename + '_q.svg')))

indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPath(out_dir, filename + "_q.png"))
print(checkImageSimilarity('%s' % (filename + '_q.png')))

print("   OK")
