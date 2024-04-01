import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

smarts_in = "([#8:1].[#6:2])>>([#8:1].[#6:2])"
rxn1 = indigo.loadReactionSmarts(smarts_in)
if rxn1.countReactants() == 1 and rxn1.countProducts() == 1:
    print("SMARTS component-level grouping load ok")
else:
    print("SMARTS component-level grouping load failed")
    print("rxn1.countReactants()=%s" % rxn1.countReactants())
    print("rxn1.countProducts()=%s" % rxn1.countProducts())
smarts_out = rxn1.smarts()
if smarts_in == smarts_out:
    print("SMARTS component-level grouping save ok")
else:
    print("SMARTS component-level grouping save failed")
    print("smart_in=%s" % smarts_in)
    print("smart_ou=%s" % smarts_out)
