import sys
import errno

sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = joinPathPy("out/smarts", __file__)

if not os.path.exists(out_dir):
    try:
        os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
idx = 1
render_rarefiy_index = 0
render_rerefiy_coeff = 20
for line in open(joinPathPy("molecules/smarts.sma", __file__)):
    line = line.rstrip()
    print("%d: %s" % (idx, line))
    render_rarefiy_index += 1
    try:
        mol = indigo.loadSmarts(line)
        mol.layout()
        indigo.setOption("render-comment", line)
        indigo.setOption("render-comment-offset", 40)
        indigo.setOption("render-comment-color", "0, 0.3, 0.5")
        indigo.setOption("render-output-format", "svg")
        renderer.renderToFile(mol, "%s/%04d.svg" % (out_dir, idx))
        print(checkImageSimilarity('smarts/%04d.svg' % idx))
        indigo.setOption("render-output-format", "png")
        renderer.renderToFile(mol, "%s/%04d.png" % (out_dir, idx))
        print(checkImageSimilarity('smarts/%04d.png' % idx))

    except IndigoException as e:
        print("  %s" % (getIndigoExceptionText(e)))
    idx += 1
if isIronPython():
    renderer.Dispose()