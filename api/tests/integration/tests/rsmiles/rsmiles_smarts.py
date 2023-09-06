import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


rxn1 = indigo.loadReactionSmarts("([#8:1].[#6:2])>>([#8:1].[#6:2])")
assert rxn1.countReactants() == 1
assert rxn1.countProducts() == 1
print("SMARTS component-level grouping load ok")
