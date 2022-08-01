import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

t = indigo.loadMoleculeFromFile(joinPathPy("molecules/t_795.mol", __file__))
q = indigo.loadMoleculeFromFile(joinPathPy("molecules/q_42.mol", __file__))

print(indigo.exactMatch(q, t, "ALL") is not None)
print(indigo.exactMatch(q, t, "ALL -FRA") is not None)

print(indigo.exactMatch(q, t, "") is not None)
print(indigo.exactMatch(q, t, "-FRA") is not None)
