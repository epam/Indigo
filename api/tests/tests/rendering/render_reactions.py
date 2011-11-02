import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)
if not os.path.exists("out"):
   os.makedirs("out")
def renderRxnfile (filename, outfile):
  rxn = indigo.loadReactionFromFile(filename)
  indigo.setOption("render-output-format", "svg")
  indigo.setOption("render-label-mode", "hetero")
  indigo.setOption("render-bond-length", 40.)
  renderer.renderToFile(rxn, "out/" + outfile + ".svg")
  rxn2 = indigo.loadReaction(rxn.smiles())
  #indigo.setOption("render-output-format", "png")
  renderer.renderToFile(rxn2, "out/" + outfile + "-smiles.svg")
  
renderRxnfile("../../../../rxnfiles/adama_reaction.rxn", "adama_reaction")
renderRxnfile("../../../../rxnfiles/epoxy.rxn", "epoxy")
print "Done"
