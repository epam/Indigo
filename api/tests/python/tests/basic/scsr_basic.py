import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("molfile-saving-mode", "3000")

templates = indigo.loadMoleculeFromFile(joinPath("molecules/BIOVIADraw_all_templates.mol"))

indigo.setOption("ignore-stereochemistry-errors", "true")

for item in indigo.iterateSDFile(joinPath("molecules/peptides.sdf.gz")):

    mol = item.clone()

    print("****** transform CTAB to SCSR structure ********")
    item.transformCTABtoSCSR(templates)

    print("****** transform SCSR to Full CTAB structure ********")
    item.transformSCSRtoCTAB()
    item.layout()

    print("****** check match after direct and back transformation ******")
    print(mol.grossFormula())
    print(item.grossFormula())
    match1 = indigo.exactMatch(mol, item, 'ALL')

    mol.aromatize()
    item.aromatize()

    match2 = indigo.exactMatch(mol, item, 'ALL')
 
    if match1 or match2:
        print("Matched")
    else:
        print("Not matched")
