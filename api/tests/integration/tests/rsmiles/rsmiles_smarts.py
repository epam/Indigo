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
assert rxn1.countReactants() == 1
assert rxn1.countProducts() == 1
print("SMARTS component-level grouping load ok")
smarts_out = rxn1.smarts()
assert smarts_in == smarts_out
print("SMARTS component-level grouping save ok")
