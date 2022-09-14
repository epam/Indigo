import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("pka-model", "simple")

# indigo.setOption("pka-model", "advanced")
# indigo.setOption("pKa-model-level", 5)
# indigo.setOption("pKa-model-min-level", 0)
# for m in indigo.iterateSDFile(dataPath("molecules/pka/PkaModel.sdf")):
m = indigo.loadMolecule("[NH+]C=N")
print(m.canonicalSmiles())
for atom in m.iterateAtoms():
    print(atom.index(), atom.symbol(), atom.getAcidPkaValue(), atom.getBasicPkaValue())
print(m.crippenPka())


