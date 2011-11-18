import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists(joinPath("out")):
   os.makedirs(joinPath("out"))
   
def renderRxnfile (filename, outfile):
  rxn = indigo.loadReactionFromFile(filename)
  indigo.setOption("render-output-format", "svg")
  indigo.setOption("render-label-mode", "hetero")
  indigo.setOption("render-bond-length", 40.)
  renderer.renderToFile(rxn, joinPath("out/%s.svg" % (outfile)))
  print("\nout/%s.svg\n" % (outfile))
  with open(joinPath("out/%s.svg" % (outfile))) as f:
    print(f.read())
  rxn2 = indigo.loadReaction(rxn.smiles())
  renderer.renderToFile(rxn2, joinPath("out/%s-smiles.svg" % (outfile)))
  print("\nout/%s-smiles.svg\n" % (outfile))
  with open(joinPath("out/%s-smiles.svg" % (outfile))) as f:
    print(f.read())
    
renderRxnfile(joinPath("reactions/adama_reaction.rxn"), "adama_reaction")
renderRxnfile(joinPath("reactions/epoxy.rxn"), "epoxy")
print("Done")