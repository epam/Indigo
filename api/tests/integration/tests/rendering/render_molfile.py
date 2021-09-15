import sys, os
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

out_dir = joinPathPy("out/molfile", __file__)

if not os.access(out_dir, os.F_OK):
    try:
        os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
    
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", True)
indigo.setOption("ignore-stereochemistry-errors", True)

for root, dirnames, filenames in os.walk(joinPathPy("molecules/molfile", __file__)):
    filenames.sort()
    for filename in filenames:
        print("%s: " % filename)
        try:
            mol = indigo.loadMoleculeFromFile(joinPathPy(os.path.join(root, filename), __file__))
            indigo.setOption("render-output-format", "svg")
            renderer.renderToFile(mol, joinPathPy(os.path.join(out_dir, filename + "_t.svg"), __file__))
            print(checkImageSimilarity('molfile/%s' % (filename + '_t.svg')))
            indigo.setOption("render-output-format", "png")
            renderer.renderToFile(mol, joinPathPy(os.path.join(out_dir, filename + "_t.png"), __file__))
            print(checkImageSimilarity('molfile/%s' % (filename + '_t.png')))
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

        try:
            mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
            indigo.setOption("render-output-format", "svg")
            renderer.renderToFile(mol, joinPathPy(os.path.join(out_dir, filename + "_q.svg"), __file__))
            print(checkImageSimilarity('molfile/%s' % (filename + '_q.svg')))
            indigo.setOption("render-output-format", "png")
            renderer.renderToFile(mol, joinPathPy(os.path.join(out_dir, filename + "_q.png"), __file__))
            print(checkImageSimilarity('molfile/%s' % (filename + '_q.png')))
            print("   OK")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
