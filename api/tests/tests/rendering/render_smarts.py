import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = joinPath("out_smarts")
if not os.path.exists(out_dir):
   os.makedirs(out_dir)
indigo.setOption("render-background-color", "255, 255, 255")
indigo.setOption("render-coloring", "true")
idx = 1
for line in open(joinPath("molecules/smarts.sma")):
   line = line.rstrip()
   print("%d: %s" % (idx, line))
   try:
      mol = indigo.loadSmarts(line)
      mol.layout()
      indigo.setOption("render-comment", line)
      indigo.setOption("render-comment-offset", 40)
      indigo.setOption("render-comment-color", "0, 0.3, 0.5")
      indigo.setOption("render-output-format", "png")
      renderer.renderToFile(mol, "%s/%04d.png" % (out_dir, idx))
      indigo.setOption("render-output-format", "svg")
      print("\n%s/%04d.svg:\n" % (out_dir, idx))
      print(renderer.renderToBuffer(mol))
      
   except IndigoException, e:
      print("  %s" % (getIndigoExceptionText(e)))
   idx += 1
   
   