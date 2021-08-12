import sys, os
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

out_dir = joinPathPy("out/layout", __file__)

if not os.path.exists(out_dir):
    try:
        os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
indigo.setOption("render-output-format", "png")

images_to_check = []

for root, dirnames, filenames in os.walk(joinPathPy("molecules/layout", __file__)):
    filenames.sort()
    for filename in filenames:
        print("%s: " % filename)
        try:
            m = indigo.loadMoleculeFromFile(joinPathPy(os.path.join(root, filename), __file__))

            for idx in range(3):
                print(m.canonicalSmiles())
                name = filename + "_%d.png" % idx
                m.saveMolfile(joinPathPy(os.path.join(out_dir, name + ".mol"), __file__))

                renderer.renderToFile(m, joinPathPy(os.path.join(out_dir, name), __file__))
                images_to_check.append('layout/' + name)
                m = indigo.loadMolecule(m.molfile())
                m.layout()

            print("  rendered")
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

print("Checking image similarities:")
for name in images_to_check:
    print(checkImageSimilarity(name))
if isIronPython():
    renderer.Dispose()