import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("molfile-saving-mode", "3000")

templates = indigo.loadMoleculeFromFile(joinPath("molecules/BIOVIADraw_all_templates.mol"))

indigo.setOption("ignore-stereochemistry-errors", "true")
m = indigo.loadMoleculeFromFile(joinPath("molecules/SCSR_test.mol"))
tind = m.findTemplate("ala")
print("Template ala found: %d" % tind)
tind = m.findTemplate("val")
print("Template val found: %d" % tind)
m.removeTemplate("ala")
ind_ala = m.findTemplate("ala")
if ind_ala == 0:
   print("Template ala removed.")
ind_Ala = m.addTemplate(templates, "Ala")
tind = m.findTemplate("Ala")
if ind_Ala == tind:
   print("Template Ala added %d" % tind)
