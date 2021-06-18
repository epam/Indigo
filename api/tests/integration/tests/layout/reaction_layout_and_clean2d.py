import sys
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")

saver = indigo.writeFile(joinPath("out", "reaction_layout_and_clean2d.rdf"))

ref_path_layout = getRefFilepath("reaction_layout.rdf")
ref_path_clean2d = getRefFilepath("reaction_clean2d.rdf")
ref_layout = indigo.iterateRDFile(ref_path_layout)
ref_clean2d = indigo.iterateRDFile(ref_path_clean2d)

print("**** Test reaction leayout and clean2d layout *****")

for idx, item in enumerate(indigo.iterateRDFile(joinPath("reactions", "reaction_layout_and_clean2d.rdf"))):
    try:
        rxn = item.clone()
        rxn.layout()
        res = reactionLayoutDiff(indigo, rxn, ref_layout.at(idx).rawData(), ref_is_file=False)
        print("  Item #{}: Result of layout: {}".format(idx, res))
        saver.rdfAppend(rxn)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))

for idx, item in enumerate(indigo.iterateRDFile(joinPath("reactions", "reaction_layout_and_clean2d.rdf"))):
    try:
        rxn = item.clone()
        rxn.clean2d()
        res = reactionLayoutDiff(indigo, rxn, ref_clean2d.at(idx).rawData(), ref_is_file=False)
        print("  Item #{}: Result of clean2d: {}".format(idx, res))
        saver.rdfAppend(rxn)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
