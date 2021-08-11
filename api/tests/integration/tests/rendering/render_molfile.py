import sys, os
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

out_dir = joinPath("out/molfile")

if not os.path.exists(joinPath("out/molfile")):
    try:
        os.makedirs(joinPath("out/molfile"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
    
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("ignore-stereochemistry-errors", True)

for root, dirnames, filenames in os.walk(joinPath("molecules", "molfile")):
    filenames.sort()
    for filename in filenames:
        print("%s: " % filename)
        try:
            mol = indigo.loadMoleculeFromFile(joinPath(root, filename))
            indigo.setOption("render-output-format", "svg")
            renderer.renderToFile(mol, joinPath(out_dir, filename + "_t.svg"))
            print(checkImageSimilarity('molfile/%s' % (filename + '_t.svg')))
            indigo.setOption("render-output-format", "png")
            renderer.renderToFile(mol, joinPath(out_dir, filename + "_t.png"))
            print(checkImageSimilarity('molfile/%s' % (filename + '_t.png')))
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

        try:
            mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
            indigo.setOption("render-output-format", "svg")
            renderer.renderToFile(mol, joinPath(out_dir, filename + "_q.svg"))
            print(checkImageSimilarity('molfile/%s' % (filename + '_q.svg')))
            indigo.setOption("render-output-format", "png")
            renderer.renderToFile(mol, joinPath(out_dir, filename + "_q.png"))
            print(checkImageSimilarity('molfile/%s' % (filename + '_q.png')))
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))
renderer.Dispose()